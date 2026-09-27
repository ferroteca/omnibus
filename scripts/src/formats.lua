-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   formats.lua
--
--   Rules for building formats
--
---------------------------------------------------------------------------
function formatsProject(_target, _subtarget)
project "formats"
	uuid (os.uuid("formats-" .. _target .."_" .. _subtarget))
	kind (LIBTYPE)
	targetsubdir(_target .."_" .. _subtarget)
	addprojectflags()

	options {
		"ArchiveSplit",
	}

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR,
		ext_includedir("flac"),
		ext_includedir("zlib"),
	}

	files {
		MAME_DIR .. "src/lib/formats/all.cpp",
		MAME_DIR .. "src/lib/formats/all.h",

		MAME_DIR .. "src/lib/formats/imageutl.cpp",
		MAME_DIR .. "src/lib/formats/imageutl.h",

		MAME_DIR .. "src/lib/formats/aiffile.cpp",
		MAME_DIR .. "src/lib/formats/aiffile.h",
		MAME_DIR .. "src/lib/formats/cassimg.cpp",
		MAME_DIR .. "src/lib/formats/cassimg.h",
		MAME_DIR .. "src/lib/formats/flacfile.cpp",
		MAME_DIR .. "src/lib/formats/flacfile.h",
		MAME_DIR .. "src/lib/formats/wavfile.cpp",
		MAME_DIR .. "src/lib/formats/wavfile.h",

		MAME_DIR .. "src/lib/formats/flopimg.cpp",
		MAME_DIR .. "src/lib/formats/flopimg.h",
		MAME_DIR .. "src/lib/formats/flopimg_legacy.cpp",
		MAME_DIR .. "src/lib/formats/flopimg_legacy.h",

		MAME_DIR .. "src/lib/formats/cqm_dsk.cpp",
		MAME_DIR .. "src/lib/formats/cqm_dsk.h",
		MAME_DIR .. "src/lib/formats/dsk_dsk.cpp",
		MAME_DIR .. "src/lib/formats/dsk_dsk.h",
		MAME_DIR .. "src/lib/formats/ipf_dsk.cpp",
		MAME_DIR .. "src/lib/formats/ipf_dsk.h",
		MAME_DIR .. "src/lib/formats/td0_dsk.cpp",
		MAME_DIR .. "src/lib/formats/td0_dsk.h",
		MAME_DIR .. "src/lib/formats/hxchfe_dsk.cpp",
		MAME_DIR .. "src/lib/formats/hxchfe_dsk.h",
		MAME_DIR .. "src/lib/formats/hxcmfm_dsk.cpp",
		MAME_DIR .. "src/lib/formats/hxcmfm_dsk.h",
		MAME_DIR .. "src/lib/formats/mfi_dsk.cpp",
		MAME_DIR .. "src/lib/formats/mfi_dsk.h",
		MAME_DIR .. "src/lib/formats/imd_dsk.cpp",
		MAME_DIR .. "src/lib/formats/imd_dsk.h",
		MAME_DIR .. "src/lib/formats/upd765_dsk.cpp",
		MAME_DIR .. "src/lib/formats/upd765_dsk.h",
		MAME_DIR .. "src/lib/formats/pc_dsk.cpp",
		MAME_DIR .. "src/lib/formats/pc_dsk.h",
		MAME_DIR .. "src/lib/formats/d88_dsk.cpp",
		MAME_DIR .. "src/lib/formats/d88_dsk.h",
		MAME_DIR .. "src/lib/formats/dfi_dsk.cpp",
		MAME_DIR .. "src/lib/formats/dfi_dsk.h",
		MAME_DIR .. "src/lib/formats/fdi_dsk.cpp",
		MAME_DIR .. "src/lib/formats/rpk.cpp",
		MAME_DIR .. "src/lib/formats/rpk.h",
		MAME_DIR .. "src/lib/formats/86f_dsk.cpp",
		MAME_DIR .. "src/lib/formats/86f_dsk.h",

		MAME_DIR .. "src/lib/formats/fsmgr.h",
		MAME_DIR .. "src/lib/formats/fsmgr.cpp",
		MAME_DIR .. "src/lib/formats/fsblk.h",
		MAME_DIR .. "src/lib/formats/fsblk.cpp",
		MAME_DIR .. "src/lib/formats/fsblk_multi.h",
		MAME_DIR .. "src/lib/formats/fsblk_vec.h",
		MAME_DIR .. "src/lib/formats/fsblk_vec.cpp",
		MAME_DIR .. "src/lib/formats/fs_unformatted.h",
		MAME_DIR .. "src/lib/formats/fs_unformatted.cpp",
		MAME_DIR .. "src/lib/formats/fsmeta.h",
		MAME_DIR .. "src/lib/formats/fsmeta.cpp",
	}

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "2D_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/2d_dsk.cpp",
		MAME_DIR.. "src/lib/formats/2d_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "A26_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/a26_cas.cpp",
		MAME_DIR.. "src/lib/formats/a26_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "A5105_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/a5105_dsk.cpp",
		MAME_DIR.. "src/lib/formats/a5105_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ABC800_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/abc800_dsk.cpp",
		MAME_DIR.. "src/lib/formats/abc800_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ABC800I_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/abc800i_dsk.cpp",
		MAME_DIR.. "src/lib/formats/abc800i_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ABC1600_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/abc1600_dsk.cpp",
		MAME_DIR.. "src/lib/formats/abc1600_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ABCFD2_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/abcfd2_dsk.cpp",
		MAME_DIR.. "src/lib/formats/abcfd2_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ACE_TAP") then
	files {
		MAME_DIR.. "src/lib/formats/ace_tap.cpp",
		MAME_DIR.. "src/lib/formats/ace_tap.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ACORN_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/acorn_dsk.cpp",
		MAME_DIR.. "src/lib/formats/acorn_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ADAM_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/adam_cas.cpp",
		MAME_DIR.. "src/lib/formats/adam_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ADAM_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/adam_dsk.cpp",
		MAME_DIR.. "src/lib/formats/adam_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "AFS_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/afs_dsk.cpp",
		MAME_DIR.. "src/lib/formats/afs_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "AGAT840K_HLE_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/agat840k_hle_dsk.cpp",
		MAME_DIR.. "src/lib/formats/agat840k_hle_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "AIM_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/aim_dsk.cpp",
		MAME_DIR.. "src/lib/formats/aim_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "AMI_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ami_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ami_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "AP2_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ap2_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ap2_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "APD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/apd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/apd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "APF_APT") then
	files {
		MAME_DIR.. "src/lib/formats/apf_apt.cpp",
		MAME_DIR.. "src/lib/formats/apf_apt.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "APOLLO_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/apollo_dsk.cpp",
		MAME_DIR.. "src/lib/formats/apollo_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "APPLIX_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/applix_dsk.cpp",
		MAME_DIR.. "src/lib/formats/applix_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "APRICOTPC_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/apricotpc_dsk.cpp",
		MAME_DIR.. "src/lib/formats/apricotpc_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "APRIDISK") then
	files {
		MAME_DIR.. "src/lib/formats/apridisk.cpp",
		MAME_DIR.. "src/lib/formats/apridisk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "AP_DSK35") then
	files {
		MAME_DIR.. "src/lib/formats/ap_dsk35.cpp",
		MAME_DIR.. "src/lib/formats/ap_dsk35.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "AQUARIUS_CAQ") then
	files {
		MAME_DIR.. "src/lib/formats/aquarius_caq.cpp",
		MAME_DIR.. "src/lib/formats/aquarius_caq.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ASST128_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/asst128_dsk.cpp",
		MAME_DIR.. "src/lib/formats/asst128_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "AS_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/as_dsk.cpp",
		MAME_DIR.. "src/lib/formats/as_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ATARI_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/atari_dsk.cpp",
		MAME_DIR.. "src/lib/formats/atari_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ATOM_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/atom_dsk.cpp",
		MAME_DIR.. "src/lib/formats/atom_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ATOM_TAP") then
	files {
		MAME_DIR.. "src/lib/formats/atom_tap.cpp",
		MAME_DIR.. "src/lib/formats/atom_tap.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "BASICDSK") then
	files {
		MAME_DIR.. "src/lib/formats/basicdsk.cpp",
		MAME_DIR.. "src/lib/formats/basicdsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "BK0010_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/bk0010_dsk.cpp",
		MAME_DIR.. "src/lib/formats/bk0010_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "BW12_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/bw12_dsk.cpp",
		MAME_DIR.. "src/lib/formats/bw12_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "BW2_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/bw2_dsk.cpp",
		MAME_DIR.. "src/lib/formats/bw2_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "C3040_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/c3040_dsk.cpp",
		MAME_DIR.. "src/lib/formats/c3040_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "C4040_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/c4040_dsk.cpp",
		MAME_DIR.. "src/lib/formats/c4040_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "C8280_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/c8280_dsk.cpp",
		MAME_DIR.. "src/lib/formats/c8280_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CAMPLYNX_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/camplynx_cas.cpp",
		MAME_DIR.. "src/lib/formats/camplynx_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CAMPLYNX_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/camplynx_dsk.cpp",
		MAME_DIR.. "src/lib/formats/camplynx_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CBM_CRT") then
	files {
		MAME_DIR.. "src/lib/formats/cbm_crt.cpp",
		MAME_DIR.. "src/lib/formats/cbm_crt.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CBM_TAP") then
	files {
		MAME_DIR.. "src/lib/formats/cbm_tap.cpp",
		MAME_DIR.. "src/lib/formats/cbm_tap.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CCVF_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ccvf_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ccvf_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CGENIE_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/cgenie_dsk.cpp",
		MAME_DIR.. "src/lib/formats/cgenie_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CGEN_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/cgen_cas.cpp",
		MAME_DIR.. "src/lib/formats/cgen_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "COCO_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/coco_cas.cpp",
		MAME_DIR.. "src/lib/formats/coco_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "COCO_RAWDSK") then
	files {
		MAME_DIR.. "src/lib/formats/coco_rawdsk.cpp",
		MAME_DIR.. "src/lib/formats/coco_rawdsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "COMX35_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/comx35_dsk.cpp",
		MAME_DIR.. "src/lib/formats/comx35_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CONCEPT_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/concept_dsk.cpp",
		MAME_DIR.. "src/lib/formats/concept_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "COUPEDSK") then
	files {
		MAME_DIR.. "src/lib/formats/coupedsk.cpp",
		MAME_DIR.. "src/lib/formats/coupedsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CPIS_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/cpis_dsk.cpp",
		MAME_DIR.. "src/lib/formats/cpis_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CSW_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/csw_cas.cpp",
		MAME_DIR.. "src/lib/formats/csw_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "D64_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/d64_dsk.cpp",
		MAME_DIR.. "src/lib/formats/d64_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "D71_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/d71_dsk.cpp",
		MAME_DIR.. "src/lib/formats/d71_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "D80_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/d80_dsk.cpp",
		MAME_DIR.. "src/lib/formats/d80_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "D81_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/d81_dsk.cpp",
		MAME_DIR.. "src/lib/formats/d81_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "D82_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/d82_dsk.cpp",
		MAME_DIR.. "src/lib/formats/d82_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "DCP_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/dcp_dsk.cpp",
		MAME_DIR.. "src/lib/formats/dcp_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "DIM_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/dim_dsk.cpp",
		MAME_DIR.. "src/lib/formats/dim_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "DIP_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/dip_dsk.cpp",
		MAME_DIR.. "src/lib/formats/dip_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "DMK_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/dmk_dsk.cpp",
		MAME_DIR.. "src/lib/formats/dmk_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "DS9_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ds9_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ds9_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SDF_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/sdf_dsk.cpp",
		MAME_DIR.. "src/lib/formats/sdf_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "EP64_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ep64_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ep64_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "DMV_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/dmv_dsk.cpp",
		MAME_DIR.. "src/lib/formats/dmv_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "DVK_MX_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/dvk_mx_dsk.cpp",
		MAME_DIR.. "src/lib/formats/dvk_mx_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ESQ16_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/esq16_dsk.cpp",
		MAME_DIR.. "src/lib/formats/esq16_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ESQ8_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/esq8_dsk.cpp",
		MAME_DIR.. "src/lib/formats/esq8_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "EXCALI64_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/excali64_dsk.cpp",
		MAME_DIR.. "src/lib/formats/excali64_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FC100_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/fc100_cas.cpp",
		MAME_DIR.. "src/lib/formats/fc100_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FDD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/fdd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/fdd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FL1_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/fl1_dsk.cpp",
		MAME_DIR.. "src/lib/formats/fl1_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FLEX_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/flex_dsk.cpp",
		MAME_DIR.. "src/lib/formats/flex_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "CP68_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/cp68_dsk.cpp",
		MAME_DIR.. "src/lib/formats/cp68_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FDOS_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/fdos_dsk.cpp",
		MAME_DIR.. "src/lib/formats/fdos_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "UNIFLEX_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/uniflex_dsk.cpp",
		MAME_DIR.. "src/lib/formats/uniflex_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FM7_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/fm7_cas.cpp",
		MAME_DIR.. "src/lib/formats/fm7_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FMSX_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/fmsx_cas.cpp",
		MAME_DIR.. "src/lib/formats/fmsx_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FMTOWNS_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/fmtowns_dsk.cpp",
		MAME_DIR.. "src/lib/formats/fmtowns_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FSD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/fsd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/fsd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FZ1_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/fz1_dsk.cpp",
		MAME_DIR.. "src/lib/formats/fz1_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "G64_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/g64_dsk.cpp",
		MAME_DIR.. "src/lib/formats/g64_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "GTP_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/gtp_cas.cpp",
		MAME_DIR.. "src/lib/formats/gtp_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "GUAB_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/guab_dsk.cpp",
		MAME_DIR.. "src/lib/formats/guab_dsk.h",
	}
end

--------------------------------------------------
--
--@src/lib/formats/h17disk.h,FORMATS["H17D_DSK"] = true
--------------------------------------------------

if opt_tool(FORMATS, "H17D_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/h17disk.cpp",
		MAME_DIR.. "src/lib/formats/h17disk.h",
	}
end

--------------------------------------------------
--
--@src/lib/formats/h8_cas.h,FORMATS["H8_CAS"] = true
--------------------------------------------------

if opt_tool(FORMATS, "H8_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/h8_cas.cpp",
		MAME_DIR.. "src/lib/formats/h8_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "HECTOR_MINIDISC") then
	files {
		MAME_DIR.. "src/lib/formats/hector_minidisc.cpp",
		MAME_DIR.. "src/lib/formats/hector_minidisc.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "HECT_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/hect_dsk.cpp",
		MAME_DIR.. "src/lib/formats/hect_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "HECT_TAP") then
	files {
		MAME_DIR.. "src/lib/formats/hect_tap.cpp",
		MAME_DIR.. "src/lib/formats/hect_tap.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "HTI_TAP") then
	files {
		MAME_DIR.. "src/lib/formats/hti_tape.cpp",
		MAME_DIR.. "src/lib/formats/hti_tape.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "HP300_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/hp300_dsk.cpp",
		MAME_DIR.. "src/lib/formats/hp300_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "HPI_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/hpi_dsk.cpp",
		MAME_DIR.. "src/lib/formats/hpi_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "HP_IPC_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/hp_ipc_dsk.cpp",
		MAME_DIR.. "src/lib/formats/hp_ipc_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "IDPART_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/idpart_dsk.cpp",
		MAME_DIR.. "src/lib/formats/idpart_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "IMG_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/img_dsk.cpp",
		MAME_DIR.. "src/lib/formats/img_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "IQ151_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/iq151_dsk.cpp",
		MAME_DIR.. "src/lib/formats/iq151_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ITT3030_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/itt3030_dsk.cpp",
		MAME_DIR.. "src/lib/formats/itt3030_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "JUKU_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/juku_dsk.cpp",
		MAME_DIR.. "src/lib/formats/juku_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "JVC_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/jvc_dsk.cpp",
		MAME_DIR.. "src/lib/formats/jvc_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "LW30_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/lw30_dsk.cpp",
		MAME_DIR.. "src/lib/formats/lw30_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "OS9_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/os9_dsk.cpp",
		MAME_DIR.. "src/lib/formats/os9_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "JFD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/jfd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/jfd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "KAYPRO_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/kaypro_dsk.cpp",
		MAME_DIR.. "src/lib/formats/kaypro_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "KC85_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/kc85_dsk.cpp",
		MAME_DIR.. "src/lib/formats/kc85_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "KC_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/kc_cas.cpp",
		MAME_DIR.. "src/lib/formats/kc_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "KIM1_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/kim1_cas.cpp",
		MAME_DIR.. "src/lib/formats/kim1_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "LVIV_LVT") then
	files {
		MAME_DIR.. "src/lib/formats/lviv_lvt.cpp",
		MAME_DIR.. "src/lib/formats/lviv_lvt.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "M20_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/m20_dsk.cpp",
		MAME_DIR.. "src/lib/formats/m20_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "M5_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/m5_dsk.cpp",
		MAME_DIR.. "src/lib/formats/m5_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "MBEE_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/mbee_cas.cpp",
		MAME_DIR.. "src/lib/formats/mbee_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "MDOS_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/mdos_dsk.cpp",
		MAME_DIR.. "src/lib/formats/mdos_dsk.h",
	}
end

--------------------------------------------------
--
--@src/lib/formats/mfm_hd.h,FORMATS["MFM_HD"] = true
--------------------------------------------------

if opt_tool(FORMATS, "MFM_HD") then
	files {
		MAME_DIR.. "src/lib/formats/mfm_hd.cpp",
		MAME_DIR.. "src/lib/formats/mfm_hd.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "MM_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/mm_dsk.cpp",
		MAME_DIR.. "src/lib/formats/mm_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "MS0515_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ms0515_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ms0515_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "MSX_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/msx_dsk.cpp",
		MAME_DIR.. "src/lib/formats/msx_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "MTX_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/mtx_dsk.cpp",
		MAME_DIR.. "src/lib/formats/mtx_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "MZ_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/mz_cas.cpp",
		MAME_DIR.. "src/lib/formats/mz_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "NABUPC_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/nabupc_dsk.cpp",
		MAME_DIR.. "src/lib/formats/nabupc_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "NANOS_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/nanos_dsk.cpp",
		MAME_DIR.. "src/lib/formats/nanos_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "NASCOM_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/nascom_dsk.cpp",
		MAME_DIR.. "src/lib/formats/nascom_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "NASLITE_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/naslite_dsk.cpp",
		MAME_DIR.. "src/lib/formats/naslite_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "NES_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/nes_dsk.cpp",
		MAME_DIR.. "src/lib/formats/nes_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "NFD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/nfd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/nfd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "OPD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/opd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/opd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ORAO_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/orao_cas.cpp",
		MAME_DIR.. "src/lib/formats/orao_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ORIC_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/oric_dsk.cpp",
		MAME_DIR.. "src/lib/formats/oric_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ORIC_TAP") then
	files {
		MAME_DIR.. "src/lib/formats/oric_tap.cpp",
		MAME_DIR.. "src/lib/formats/oric_tap.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "IBMXDF_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ibmxdf_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ibmxdf_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "P2000T_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/p2000t_cas.cpp",
		MAME_DIR.. "src/lib/formats/p2000t_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PASTI_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/pasti_dsk.cpp",
		MAME_DIR.. "src/lib/formats/pasti_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PC98FDI_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/pc98fdi_dsk.cpp",
		MAME_DIR.. "src/lib/formats/pc98fdi_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PC98_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/pc98_dsk.cpp",
		MAME_DIR.. "src/lib/formats/pc98_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PHC25_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/phc25_cas.cpp",
		MAME_DIR.. "src/lib/formats/phc25_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PK8020_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/pk8020_dsk.cpp",
		MAME_DIR.. "src/lib/formats/pk8020_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PMD_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/pmd_cas.cpp",
		MAME_DIR.. "src/lib/formats/pmd_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "POLY_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/poly_dsk.cpp",
		MAME_DIR.. "src/lib/formats/poly_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PPG_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ppg_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ppg_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PRIMOPTP") then
	files {
		MAME_DIR.. "src/lib/formats/primoptp.cpp",
		MAME_DIR.. "src/lib/formats/primoptp.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "PYLDIN_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/pyldin_dsk.cpp",
		MAME_DIR.. "src/lib/formats/pyldin_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "QL_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ql_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ql_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "RC759_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/rc759_dsk.cpp",
		MAME_DIR.. "src/lib/formats/rc759_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "RK_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/rk_cas.cpp",
		MAME_DIR.. "src/lib/formats/rk_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ROLAND_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/roland_dsk.cpp",
		MAME_DIR.. "src/lib/formats/roland_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "RX01_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/rx01_dsk.cpp",
		MAME_DIR.. "src/lib/formats/rx01_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "RX50_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/rx50_dsk.cpp",
		MAME_DIR.. "src/lib/formats/rx50_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "S900_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/s900_dsk.cpp",
		MAME_DIR.. "src/lib/formats/s900_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SAP_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/sap_dsk.cpp",
		MAME_DIR.. "src/lib/formats/sap_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SC3000_BIT") then
	files {
		MAME_DIR.. "src/lib/formats/sc3000_bit.cpp",
		MAME_DIR.. "src/lib/formats/sc3000_bit.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SCL_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/scl_dsk.cpp",
		MAME_DIR.. "src/lib/formats/scl_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SDD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/sdd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/sdd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SF7000_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/sf7000_dsk.cpp",
		MAME_DIR.. "src/lib/formats/sf7000_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SMX_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/smx_dsk.cpp",
		MAME_DIR.. "src/lib/formats/smx_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SOL_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/sol_cas.cpp",
		MAME_DIR.. "src/lib/formats/sol_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SORC_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/sorc_cas.cpp",
		MAME_DIR.. "src/lib/formats/sorc_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SORC_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/sorc_dsk.cpp",
		MAME_DIR.. "src/lib/formats/sorc_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SORD_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/sord_cas.cpp",
		MAME_DIR.. "src/lib/formats/sord_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SPC1000_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/spc1000_cas.cpp",
		MAME_DIR.. "src/lib/formats/spc1000_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ST_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/st_dsk.cpp",
		MAME_DIR.. "src/lib/formats/st_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SVI_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/svi_cas.cpp",
		MAME_DIR.. "src/lib/formats/svi_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SVI_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/svi_dsk.cpp",
		MAME_DIR.. "src/lib/formats/svi_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "SWD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/swd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/swd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TANDY2K_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/tandy2k_dsk.cpp",
		MAME_DIR.. "src/lib/formats/tandy2k_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "THOM_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/thom_cas.cpp",
		MAME_DIR.. "src/lib/formats/thom_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "THOM_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/thom_dsk.cpp",
		MAME_DIR.. "src/lib/formats/thom_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TI99_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/ti99_dsk.cpp",
		MAME_DIR.. "src/lib/formats/ti99_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TIBDD001_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/tibdd001_dsk.cpp",
		MAME_DIR.. "src/lib/formats/tibdd001_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TIKI100_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/tiki100_dsk.cpp",
		MAME_DIR.. "src/lib/formats/tiki100_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TIM011_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/tim011_dsk.cpp",
		MAME_DIR.. "src/lib/formats/tim011_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TRD_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/trd_dsk.cpp",
		MAME_DIR.. "src/lib/formats/trd_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TRS80_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/trs80_dsk.cpp",
		MAME_DIR.. "src/lib/formats/trs80_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TRS_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/trs_cas.cpp",
		MAME_DIR.. "src/lib/formats/trs_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TVC_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/tvc_cas.cpp",
		MAME_DIR.. "src/lib/formats/tvc_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TVC_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/tvc_dsk.cpp",
		MAME_DIR.. "src/lib/formats/tvc_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "TZX_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/tzx_cas.cpp",
		MAME_DIR.. "src/lib/formats/tzx_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "UEF_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/uef_cas.cpp",
		MAME_DIR.. "src/lib/formats/uef_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "VDK_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/vdk_dsk.cpp",
		MAME_DIR.. "src/lib/formats/vdk_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "VECTOR06_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/vector06_dsk.cpp",
		MAME_DIR.. "src/lib/formats/vector06_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "VG5K_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/vg5k_cas.cpp",
		MAME_DIR.. "src/lib/formats/vg5k_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "VGI_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/vgi_dsk.cpp",
		MAME_DIR.. "src/lib/formats/vgi_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "VICTOR9K_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/victor9k_dsk.cpp",
		MAME_DIR.. "src/lib/formats/victor9k_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "VT_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/vt_cas.cpp",
		MAME_DIR.. "src/lib/formats/vt_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "VT_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/vt_dsk.cpp",
		MAME_DIR.. "src/lib/formats/vt_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_VTECH") then
	files {
		MAME_DIR.. "src/lib/formats/fs_vtech.cpp",
		MAME_DIR.. "src/lib/formats/fs_vtech.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "WD177X_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/wd177x_dsk.cpp",
		MAME_DIR.. "src/lib/formats/wd177x_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "WREN_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/wren_dsk.cpp",
		MAME_DIR.. "src/lib/formats/wren_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "X07_CAS") then
	files {
		MAME_DIR.. "src/lib/formats/x07_cas.cpp",
		MAME_DIR.. "src/lib/formats/x07_cas.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "X1_TAP") then
	files {
		MAME_DIR.. "src/lib/formats/x1_tap.cpp",
		MAME_DIR.. "src/lib/formats/x1_tap.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "XDF_DSK") then
	files {
		MAME_DIR.. "src/lib/formats/xdf_dsk.cpp",
		MAME_DIR.. "src/lib/formats/xdf_dsk.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "ZX81_P") then
	files {
		MAME_DIR.. "src/lib/formats/zx81_p.cpp",
		MAME_DIR.. "src/lib/formats/zx81_p.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_PRODOS") then
	files {
		MAME_DIR.. "src/lib/formats/fs_prodos.cpp",
		MAME_DIR.. "src/lib/formats/fs_prodos.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_FAT") then
	files {
		MAME_DIR.. "src/lib/formats/fs_fat.cpp",
		MAME_DIR.. "src/lib/formats/fs_fat.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_HPLIF") then
	files {
		MAME_DIR.. "src/lib/formats/fs_hplif.cpp",
		MAME_DIR.. "src/lib/formats/fs_hplif.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_ORIC_JASMIN") then
	files {
		MAME_DIR.. "src/lib/formats/fs_oric_jasmin.cpp",
		MAME_DIR.. "src/lib/formats/fs_oric_jasmin.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_CBMDOS") then
	files {
		MAME_DIR.. "src/lib/formats/fs_cbmdos.cpp",
		MAME_DIR.. "src/lib/formats/fs_cbmdos.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_COCO_RSDOS") then
	files {
		MAME_DIR.. "src/lib/formats/fs_coco_rsdos.cpp",
		MAME_DIR.. "src/lib/formats/fs_coco_rsdos.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_COCO_OS9") then
	files {
		MAME_DIR.. "src/lib/formats/fs_coco_os9.cpp",
		MAME_DIR.. "src/lib/formats/fs_coco_os9.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_ISIS") then
	files {
		MAME_DIR.. "src/lib/formats/fs_isis.cpp",
		MAME_DIR.. "src/lib/formats/fs_isis.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_HP98X5") then
	files {
		MAME_DIR.. "src/lib/formats/fs_hp98x5.cpp",
		MAME_DIR.. "src/lib/formats/fs_hp98x5.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if opt_tool(FORMATS, "FS_ADAM_EOS") then
	files {
		MAME_DIR.. "src/lib/formats/fs_adam_eos.cpp",
		MAME_DIR.. "src/lib/formats/fs_adam_eos.h",
	}
end

end
