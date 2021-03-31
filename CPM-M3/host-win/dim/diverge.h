/****
 *
 * 	This file forces internal CP/M names to diverge from those used
 * 	by system library routines. It was automatically generated by
 * 	massaging the output of nm.
 *
 * revisions:
 *
 *	2013-06-19 rli: diverge some things I had undiverged due to ancient
 *	  compilation strategy.
 *
 ****/

#define abrt_err cpm_abrt_err
#define alloc cpm_alloc
#define alltrue cpm_alltrue
#define autorom cpm_autorom
#define autost cpm_autost
#define backsp cpm_backsp
#define bdos cpm_bdos
#define _bdos cpm__bdos
#define bdosinit cpm_bdosinit
#define bdosrw cpm_bdosrw
#define blkindx cpm_blkindx
#define blknum cpm_blknum
#define calcext cpm_calcext
#define ccp cpm_ccp
#define chainp cpm_chainp
#define chain_sub cpm_chain_sub
#define check_cmd cpm_check_cmd
#define chkaloc cpm_chkaloc
#define chk_colon cpm_chk_colon
#define close cpm_close 
#define close_fi cpm_close_fi
#define clraloc cpm_clraloc
#define cmdfcb cpm_cmdfcb
#define cmd_file cpm_cmd_file
#define cmd_tbl cpm_cmd_tbl
#define comments cpm_comments
#define conbrk cpm_conbrk
#define conin cpm_conin
#define conout cpm_conout
#define constat cpm_constat
#define cookdout cpm_cookdout
#define copy_cmd cpm_copy_cmd
#define copyrt cpm_copyrt
#define cpy cpm_cpy
#define create cpm_create
#define crit_dsk cpm_crit_dsk
#define cr_lf cpm_cr_lf
#define cur_disk cpm_cur_disk
#define dchksum cpm_dchksum
#define decode cpm_decode
#define del cpm_del
#define delete cpm_delete
#define delim cpm_delim
#define dir_cmd cpm_dir_cmd
#define dirflag cpm_dirflag
#define dir_rd cpm_dir_rd
#define dirscan cpm_dirscan
#define dir_wr cpm_dir_wr
#define dma cpm_dma
#define do_io cpm_do_io
#define dollar cpm_dollar
#define do_phio cpm_do_phio
#define echo_cmd cpm_echo_cmd
#define end_of_file cpm_end_of_file
#define era_cmd cpm_era_cmd
#define error cpm_error
#define execute_cmd cpm_execute_cmd
#define ext_err cpm_ext_err
#define extsize cpm_extsize
#define fill_fcb cpm_fill_fcb
#define find_colon cpm_find_colon
#define first_sub cpm_first_sub
#define flushit cpm_flushit
#define free_sp cpm_free_sp
#define fsize cpm_fsize
#define gbls cpm_gbls
#define getaloc cpm_getaloc
#define getch cpm_getch
#define get_cmd cpm_get_cmd
#define get_parms cpm_get_parms
#define get_rc cpm_get_rc
#define getsize cpm_getsize
#define glb_index cpm_glb_index
#define index cpm_index
#define initexc cpm_initexc
#define last_dsk cpm_last_dsk
#define lderr1 cpm_lderr1
#define lderr2 cpm_lderr2
#define lderr3 cpm_lderr3
#define lderror cpm_lderror
#define load68k cpm_load68k
#define load_tbl cpm_load_tbl
#define load_try cpm_load_try
#define log_dsk cpm_log_dsk
#define match cpm_match
#define matchit cpm_matchit
#define morecmds cpm_morecmds
#define move cpm_move
#define msg cpm_msg
#define msg10 cpm_msg10
#define msg11 cpm_msg11
#define msg12 cpm_msg12
#define msg13 cpm_msg13
#define msg2 cpm_msg2
#define msg3 cpm_msg3
#define msg4 cpm_msg4
#define msg5 cpm_msg5
#define msg6 cpm_msg6
#define msg7 cpm_msg7
#define msg8 cpm_msg8
#define msg9 cpm_msg9
#define new_ext cpm_new_ext
#define newline cpm_newline
#define openfile cpm_openfile
#define parm cpm_parm
#define pgmld cpm_pgmld
#define prompt cpm_prompt
#define prt_err cpm_prt_err
#define prt_line cpm_prt_line
#define rawconio cpm_rawconio
#define rdwrt cpm_rdwrt
#define readline cpm_readline
#define rename cpm_rename
#define ren_cmd cpm_ren_cmd
#define ro_dsk cpm_ro_dsk
#define ro_err cpm_ro_err
#define save_sub cpm_save_sub
#define scan_cmd cpm_scan_cmd
#define search cpm_search
#define seldsk cpm_seldsk
#define serial cpm_serial
#define setaloc cpm_setaloc
#define set_attr cpm_set_attr
#define setblk cpm_setblk
#define setexc cpm_setexc
#define setran cpm_setran
#define set_tpa cpm_set_tpa
#define strcmp cpm_strcmp
#define subcom cpm_subcom
#define subdma cpm_subdma
#define subfcb cpm_subfcb
#define sub_index cpm_sub_index
#define submit cpm_submit
#define submit_cmd cpm_submit_cmd
#define subprompt cpm_subprompt
#define sub_read cpm_sub_read
#define sub_user cpm_sub_user
#define swap cpm_swap
#define tabout cpm_tabout
#define tail cpm_tail
#define tmp_sel cpm_tmp_sel
#define too_many cpm_too_many
#define tpa_hp cpm_tpa_hp
#define tpa_ht cpm_tpa_ht
#define tpa_lp cpm_tpa_lp
#define tpa_lt cpm_tpa_lt
#define translate cpm_translate
#define traphndl cpm_traphndl
#define true_char cpm_true_char
#define type_cmd cpm_type_cmd
#define udiv cpm_udiv
#define user cpm_user
#define usercmd cpm_usercmd
#define user_cmd cpm_user_cmd
#define user_ptr cpm_user_ptr
#define warmboot cpm_warmboot
#define warning cpm_warning