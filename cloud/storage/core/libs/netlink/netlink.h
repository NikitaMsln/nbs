#pragma once

#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/netlink.h>

namespace NCloud::NNetlink {

using TResponseHandler = std::function<int(nl_msg*)>;

class TNetlinkSocket
{
private:
    nl_sock* Socket;
    int Family;

public:
    TNetlinkSocket(TString netlinkFamily)
    {
        Socket = nl_socket_alloc();

        if (Socket == nullptr) {
            throw TServiceError(E_FAIL) << "unable to allocate netlink socket";
        }

        if (int err = genl_connect(Socket)) {
            nl_socket_free(Socket);
            throw TServiceError(E_FAIL)
                << "unable to connect to generic netlink socket: "
                << nl_geterror(err);
        }

        Family = genl_ctrl_resolve(Socket, netlinkFamily.c_str());

        if (Family < 0) {
            nl_socket_free(Socket);
            throw TServiceError(E_FAIL)
                << "unable to resolve netlink family: "
                << nl_geterror(Family);
        }
    }

    ~TNetlinkSocket()
    {
        nl_socket_free(Socket);
    }

    int GetFamily() const
    {
        return Family;
    }

    operator nl_sock*() const
    {
        return Socket;
    }

    template <typename F>
    void SetCallback(nl_cb_type type, F func)
    {
        auto arg = std::make_unique<TResponseHandler>(std::move(func));

        if (int err = nl_socket_modify_cb(
                Socket,
                type,
                NL_CB_CUSTOM,
                TNetlinkSocket::ResponseHandler,
                arg.get()))
        {
            throw TServiceError(E_FAIL)
                << "unable to set socket callback: " << nl_geterror(err);
        }
        arg.release();
    }

    static int ResponseHandler(nl_msg* msg, void* arg)
    {
        auto func = std::unique_ptr<TResponseHandler>(
            static_cast<TResponseHandler*>(arg));

        return (*func)(msg);
    }

    void Send(nl_msg* message)
    {
        if (int err = nl_send_auto(Socket, message); err < 0) {
            throw TServiceError(E_FAIL)
                << "send error: " << nl_geterror(err);
        }
        if (int err = nl_wait_for_ack(Socket)) {
            // this is either recv error, or an actual error message received
            // from the kernel
            throw TServiceError(E_FAIL)
                << "recv error: " << nl_geterror(err);
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

class TNestedAttribute
{
private:
    nl_msg* Message;
    nlattr* Attribute;

public:
    TNestedAttribute(nl_msg* message, int attribute)
        : Message(message)
    {
        Attribute = nla_nest_start(message, attribute);
        if (!Attribute) {
            throw TServiceError(E_FAIL) << "unable to nest attribute";
        }
    }

    ~TNestedAttribute()
    {
        nla_nest_end(Message, Attribute);
    }
};

////////////////////////////////////////////////////////////////////////////////

class TNetlinkMessage
{
private:
    nl_msg* Message;

public:
    TNetlinkMessage(int family, int command, int flags = 0, int version = 0)
    {
        Message = nlmsg_alloc();
        if (Message == nullptr) {
            throw TServiceError(E_FAIL) << "unable to allocate message";
        }
        genlmsg_put(
            Message,
            NL_AUTO_PORT,
            NL_AUTO_SEQ,
            family,
            0,          // hdrlen
            flags,
            command,
            version);
    }

    ~TNetlinkMessage()
    {
        nlmsg_free(Message);
    }

    operator nl_msg*() const
    {
        return Message;
    }

    template <typename T>
    void Put(int attribute, T data)
    {
        if (int err = nla_put(Message, attribute, sizeof(T), &data)) {
            throw TServiceError(E_FAIL) << "unable to put attribute "
                << attribute << ": " << nl_geterror(err);
        }
    }

    TNestedAttribute Nest(int attribute)
    {
        return TNestedAttribute(Message, attribute);
    }
};

}