LLVM_BC(
    postgresql/src/backend/commands/async.c
    postgresql/src/backend/commands/event_trigger.c
    postgresql/src/backend/replication/walreceiver.c
    postgresql/src/backend/statistics/mcv.c
    postgresql/src/backend/tsearch/dict_simple.c
    postgresql/src/backend/tsearch/to_tsany.c
    postgresql/src/backend/tsearch/wparser.c
    postgresql/src/backend/utils/adt/int.c
    postgresql/src/backend/utils/adt/jsonb_op.c
    postgresql/src/backend/utils/adt/varlena.c
    pg_kernels.17.cpp

    NAME PgFuncs17

    SYMBOLS
    arrow_array_to_text
    arrow_array_to_text_null
    arrow_bool_int4
    arrow_btnametextcmp
    arrow_bttext_pattern_cmp
    arrow_bttext_pattern_sortsupport
    arrow_bttextcmp
    arrow_bttextnamecmp
    arrow_bttextsortsupport
    arrow_btvarstrequalimage
    arrow_byteaGetBit
    arrow_byteaGetByte
    arrow_byteaSetBit
    arrow_byteaSetByte
    arrow_bytea_bit_count
    arrow_bytea_sortsupport
    arrow_bytea_string_agg_finalfn
    arrow_bytea_string_agg_transfn
    arrow_bytea_substr
    arrow_bytea_substr_no_len
    arrow_byteacat
    arrow_byteacmp
    arrow_byteaeq
    arrow_byteage
    arrow_byteagt
    arrow_byteain
    arrow_byteale
    arrow_bytealt
    arrow_byteane
    arrow_byteaoctetlen
    arrow_byteaout
    arrow_byteaoverlay
    arrow_byteaoverlay_no_len
    arrow_byteapos
    arrow_bytearecv
    arrow_byteasend
    arrow_dsimple_init
    arrow_dsimple_lexize
    arrow_generate_series_int4_support
    arrow_get_current_ts_config
    arrow_i2toi4
    arrow_i4toi2
    arrow_in_range_int2_int2
    arrow_in_range_int2_int4
    arrow_in_range_int2_int8
    arrow_in_range_int4_int2
    arrow_in_range_int4_int4
    arrow_in_range_int4_int8
    arrow_int24div
    arrow_int24eq
    arrow_int24ge
    arrow_int24gt
    arrow_int24le
    arrow_int24lt
    arrow_int24mi
    arrow_int24mul
    arrow_int24ne
    arrow_int24pl
    arrow_int2abs
    arrow_int2and
    arrow_int2div
    arrow_int2eq
    arrow_int2ge
    arrow_int2gt
    arrow_int2in
    arrow_int2larger
    arrow_int2le
    arrow_int2lt
    arrow_int2mi
    arrow_int2mod
    arrow_int2mul
    arrow_int2ne
    arrow_int2not
    arrow_int2or
    arrow_int2out
    arrow_int2pl
    arrow_int2recv
    arrow_int2send
    arrow_int2shl
    arrow_int2shr
    arrow_int2smaller
    arrow_int2um
    arrow_int2up
    arrow_int2vectorin
    arrow_int2vectorout
    arrow_int2vectorrecv
    arrow_int2vectorsend
    arrow_int2xor
    arrow_int42div
    arrow_int42eq
    arrow_int42ge
    arrow_int42gt
    arrow_int42le
    arrow_int42lt
    arrow_int42mi
    arrow_int42mul
    arrow_int42ne
    arrow_int42pl
    arrow_int4_bool
    arrow_int4abs
    arrow_int4and
    arrow_int4div
    arrow_int4eq
    arrow_int4gcd
    arrow_int4ge
    arrow_int4gt
    arrow_int4in
    arrow_int4inc
    arrow_int4larger
    arrow_int4lcm
    arrow_int4le
    arrow_int4lt
    arrow_int4mi
    arrow_int4mod
    arrow_int4mul
    arrow_int4ne
    arrow_int4not
    arrow_int4or
    arrow_int4out
    arrow_int4pl
    arrow_int4recv
    arrow_int4send
    arrow_int4shl
    arrow_int4shr
    arrow_int4smaller
    arrow_int4um
    arrow_int4up
    arrow_int4xor
    arrow_json_string_to_tsvector
    arrow_json_string_to_tsvector_byid
    arrow_json_to_tsvector
    arrow_json_to_tsvector_byid
    arrow_jsonb_cmp
    arrow_jsonb_contained
    arrow_jsonb_contains
    arrow_jsonb_eq
    arrow_jsonb_exists
    arrow_jsonb_exists_all
    arrow_jsonb_exists_any
    arrow_jsonb_ge
    arrow_jsonb_gt
    arrow_jsonb_hash
    arrow_jsonb_hash_extended
    arrow_jsonb_le
    arrow_jsonb_lt
    arrow_jsonb_ne
    arrow_jsonb_string_to_tsvector
    arrow_jsonb_string_to_tsvector_byid
    arrow_jsonb_to_tsvector
    arrow_jsonb_to_tsvector_byid
    arrow_name_text
    arrow_nameeqtext
    arrow_namegetext
    arrow_namegttext
    arrow_nameletext
    arrow_namelttext
    arrow_namenetext
    arrow_pg_column_compression
    arrow_pg_column_size
    arrow_pg_event_trigger_table_rewrite_oid
    arrow_pg_event_trigger_table_rewrite_reason
    arrow_pg_mcv_list_in
    arrow_pg_mcv_list_out
    arrow_pg_mcv_list_recv
    arrow_pg_mcv_list_send
    arrow_pg_notification_queue_usage
    arrow_pg_notify
    arrow_pg_stat_get_wal_receiver
    arrow_phraseto_tsquery
    arrow_phraseto_tsquery_byid
    arrow_plainto_tsquery
    arrow_plainto_tsquery_byid
    arrow_replace_text
    arrow_split_part
    arrow_string_agg_finalfn
    arrow_string_agg_transfn
    arrow_text_format_nv
    arrow_text_ge
    arrow_text_gt
    arrow_text_larger
    arrow_text_le
    arrow_text_left
    arrow_text_lt
    arrow_text_name
    arrow_text_pattern_ge
    arrow_text_pattern_gt
    arrow_text_pattern_le
    arrow_text_pattern_lt
    arrow_text_reverse
    arrow_text_right
    arrow_text_smaller
    arrow_text_starts_with
    arrow_text_substr
    arrow_text_substr_no_len
    arrow_text_to_array
    arrow_text_to_array_null
    arrow_textcat
    arrow_texteq
    arrow_texteqname
    arrow_textgename
    arrow_textgtname
    arrow_textin
    arrow_textlen
    arrow_textlename
    arrow_textltname
    arrow_textne
    arrow_textnename
    arrow_textoctetlen
    arrow_textout
    arrow_textoverlay
    arrow_textoverlay_no_len
    arrow_textpos
    arrow_textrecv
    arrow_textsend
    arrow_to_hex32
    arrow_to_hex64
    arrow_to_tsquery
    arrow_to_tsquery_byid
    arrow_to_tsvector
    arrow_to_tsvector_byid
    arrow_ts_headline
    arrow_ts_headline_byid
    arrow_ts_headline_byid_opt
    arrow_ts_headline_json
    arrow_ts_headline_json_byid
    arrow_ts_headline_json_byid_opt
    arrow_ts_headline_json_opt
    arrow_ts_headline_jsonb
    arrow_ts_headline_jsonb_byid
    arrow_ts_headline_jsonb_byid_opt
    arrow_ts_headline_jsonb_opt
    arrow_ts_headline_opt
    arrow_unicode_is_normalized
    arrow_unicode_normalize_func
    arrow_unistr
    arrow_unknownin
    arrow_unknownout
    arrow_unknownrecv
    arrow_unknownsend
    arrow_websearch_to_tsquery
    arrow_websearch_to_tsquery_byid
)
