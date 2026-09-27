-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   video.lua
--
--   Rules for building video cores
--
---------------------------------------------------------------------------

files {
	MAME_DIR .. "src/devices/video/cgapal.cpp",
	MAME_DIR .. "src/devices/video/cgapal.h",
	MAME_DIR .. "src/devices/video/poly.h",
	MAME_DIR .. "src/devices/video/sprite.cpp",
	MAME_DIR .. "src/devices/video/sprite.h",
	MAME_DIR .. "src/devices/video/vector.cpp",
	MAME_DIR .. "src/devices/video/vector.h",
}

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["SEGA315_5124"] then
	files {
		MAME_DIR .. "src/devices/video/315_5124.cpp",
		MAME_DIR .. "src/devices/video/315_5124.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["SEGA315_5313"] then
	files {
		MAME_DIR .. "src/devices/video/315_5313.cpp",
		MAME_DIR .. "src/devices/video/315_5313.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["YM7101"] then
	files {
		MAME_DIR .. "src/devices/video/ym7101.cpp",
		MAME_DIR .. "src/devices/video/ym7101.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["82C425"] then
	files {
		MAME_DIR .. "src/devices/video/82c425.cpp",
		MAME_DIR .. "src/devices/video/82c425.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["AM8052"] then
	files {
		MAME_DIR .. "src/devices/video/am8052.cpp",
		MAME_DIR .. "src/devices/video/am8052.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ARIEL"] then
	files {
		MAME_DIR .. "src/devices/video/ariel.cpp",
		MAME_DIR .. "src/devices/video/ariel.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ATIRAGE"] then
	files {
		MAME_DIR .. "src/devices/video/atirage.cpp",
		MAME_DIR .. "src/devices/video/atirage.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["AVGDVG"] then
	files {
		MAME_DIR .. "src/devices/video/avgdvg.cpp",
		MAME_DIR .. "src/devices/video/avgdvg.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["BT431"] then
	files {
		MAME_DIR .. "src/devices/video/bt431.cpp",
		MAME_DIR .. "src/devices/video/bt431.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["BT450"] then
	files {
		MAME_DIR .. "src/devices/video/bt450.cpp",
		MAME_DIR .. "src/devices/video/bt450.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["BT459"] then
	files {
		MAME_DIR .. "src/devices/video/bt459.cpp",
		MAME_DIR .. "src/devices/video/bt459.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["BT45X"] then
	files {
		MAME_DIR .. "src/devices/video/bt45x.cpp",
		MAME_DIR .. "src/devices/video/bt45x.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["BT47X"] then
	files {
		MAME_DIR .. "src/devices/video/bt47x.cpp",
		MAME_DIR .. "src/devices/video/bt47x.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["BT48X"] then
	files {
		MAME_DIR .. "src/devices/video/bt48x.cpp",
		MAME_DIR .. "src/devices/video/bt48x.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["BUFSPRITE"] then
	files {
		MAME_DIR .. "src/devices/video/bufsprite.cpp",
		MAME_DIR .. "src/devices/video/bufsprite.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["CDP1861"] then
	files {
		MAME_DIR .. "src/devices/video/cdp1861.cpp",
		MAME_DIR .. "src/devices/video/cdp1861.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["CDP1862"] then
	files {
		MAME_DIR .. "src/devices/video/cdp1862.cpp",
		MAME_DIR .. "src/devices/video/cdp1862.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["CESBLIT"] then
	files {
		MAME_DIR .. "src/devices/video/cesblit.cpp",
		MAME_DIR .. "src/devices/video/cesblit.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["CRT9007"] then
	files {
		MAME_DIR .. "src/devices/video/crt9007.cpp",
		MAME_DIR .. "src/devices/video/crt9007.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["CRT9021"] then
	files {
		MAME_DIR .. "src/devices/video/crt9021.cpp",
		MAME_DIR .. "src/devices/video/crt9021.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["CRT9028"] then
	files {
		MAME_DIR .. "src/devices/video/crt9028.cpp",
		MAME_DIR .. "src/devices/video/crt9028.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["CRT9212"] then
	files {
		MAME_DIR .. "src/devices/video/crt9212.cpp",
		MAME_DIR .. "src/devices/video/crt9212.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["DL1416"] then
	files {
		MAME_DIR .. "src/devices/video/dl1416.cpp",
		MAME_DIR .. "src/devices/video/dl1416.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["DM9368"] then
	files {
		MAME_DIR .. "src/devices/video/dm9368.cpp",
		MAME_DIR .. "src/devices/video/dm9368.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["DP8350"] then
	files {
		MAME_DIR .. "src/devices/video/dp8350.cpp",
		MAME_DIR .. "src/devices/video/dp8350.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["DP8510"] then
	files {
		MAME_DIR .. "src/devices/video/dp8510.cpp",
		MAME_DIR .. "src/devices/video/dp8510.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if VIDEOS["DS8874"] then
	files {
		MAME_DIR .. "src/devices/video/ds8874.cpp",
		MAME_DIR .. "src/devices/video/ds8874.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["EF9340_1"] then
	files {
		MAME_DIR .. "src/devices/video/ef9340_1.cpp",
		MAME_DIR .. "src/devices/video/ef9340_1.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["EF9345"] then
	files {
		MAME_DIR .. "src/devices/video/ef9345.cpp",
		MAME_DIR .. "src/devices/video/ef9345.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["EF9364"] then
	files {
		MAME_DIR .. "src/devices/video/ef9364.cpp",
		MAME_DIR .. "src/devices/video/ef9364.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["EF9365"] then
	files {
		MAME_DIR .. "src/devices/video/ef9365.cpp",
		MAME_DIR .. "src/devices/video/ef9365.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["EF9369"] then
	files {
		MAME_DIR .. "src/devices/video/ef9369.cpp",
		MAME_DIR .. "src/devices/video/ef9369.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["FIXFREQ"] then
	files {
		MAME_DIR .. "src/devices/video/fixfreq.cpp",
		MAME_DIR .. "src/devices/video/fixfreq.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["GB_LCD"] then
	files {
		MAME_DIR .. "src/devices/video/gb_lcd.cpp",
		MAME_DIR .. "src/devices/video/gb_lcd.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["GBA_LCD"] then
	files {
		MAME_DIR .. "src/devices/video/gba_lcd.cpp",
		MAME_DIR .. "src/devices/video/gba_lcd.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["GF4500"] then
	files {
		MAME_DIR .. "src/devices/video/gf4500.cpp",
		MAME_DIR .. "src/devices/video/gf4500.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["GF7600GS"] then
	files {
		MAME_DIR .. "src/devices/video/gf7600gs.cpp",
		MAME_DIR .. "src/devices/video/gf7600gs.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD44102"] then
	files {
		MAME_DIR .. "src/devices/video/hd44102.cpp",
		MAME_DIR .. "src/devices/video/hd44102.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD44352"] then
	files {
		MAME_DIR .. "src/devices/video/hd44352.cpp",
		MAME_DIR .. "src/devices/video/hd44352.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD44780"] then
	files {
		MAME_DIR .. "src/devices/video/hd44780.cpp",
		MAME_DIR .. "src/devices/video/hd44780.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD61202"] then
	files {
		MAME_DIR .. "src/devices/video/hd61202.cpp",
		MAME_DIR .. "src/devices/video/hd61202.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD61602"] then
	files {
		MAME_DIR .. "src/devices/video/hd61602.cpp",
		MAME_DIR .. "src/devices/video/hd61602.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD61603"] then
	files {
		MAME_DIR .. "src/devices/video/hd61603.cpp",
		MAME_DIR .. "src/devices/video/hd61603.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD61830"] then
	files {
		MAME_DIR .. "src/devices/video/hd61830.cpp",
		MAME_DIR .. "src/devices/video/hd61830.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD63484"] then
	files {
		MAME_DIR .. "src/devices/video/hd63484.cpp",
		MAME_DIR .. "src/devices/video/hd63484.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HD66421"] then
	files {
		MAME_DIR .. "src/devices/video/hd66421.cpp",
		MAME_DIR .. "src/devices/video/hd66421.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HLCD0438"] then
	files {
		MAME_DIR .. "src/devices/video/hlcd0438.cpp",
		MAME_DIR .. "src/devices/video/hlcd0438.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HLCD0488"] then
	files {
		MAME_DIR .. "src/devices/video/hlcd0488.cpp",
		MAME_DIR .. "src/devices/video/hlcd0488.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HLCD0515"] then
	files {
		MAME_DIR .. "src/devices/video/hlcd0515.cpp",
		MAME_DIR .. "src/devices/video/hlcd0515.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HLCD0538"] then
	files {
		MAME_DIR .. "src/devices/video/hlcd0538.cpp",
		MAME_DIR .. "src/devices/video/hlcd0538.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HP1LL3"] then
	files {
		MAME_DIR .. "src/devices/video/hp1ll3.cpp",
		MAME_DIR .. "src/devices/video/hp1ll3.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HUC6202"] then
	files {
		MAME_DIR .. "src/devices/video/huc6202.cpp",
		MAME_DIR .. "src/devices/video/huc6202.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HUC6260"] then
	files {
		MAME_DIR .. "src/devices/video/huc6260.cpp",
		MAME_DIR .. "src/devices/video/huc6260.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HUC6261"] then
	files {
		MAME_DIR .. "src/devices/video/huc6261.cpp",
		MAME_DIR .. "src/devices/video/huc6261.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HUC6270"] then
	files {
		MAME_DIR .. "src/devices/video/huc6270.cpp",
		MAME_DIR .. "src/devices/video/huc6270.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HUC6271"] then
	files {
		MAME_DIR .. "src/devices/video/huc6271.cpp",
		MAME_DIR .. "src/devices/video/huc6271.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["HUC6272"] then
	files {
		MAME_DIR .. "src/devices/video/huc6272.cpp",
		MAME_DIR .. "src/devices/video/huc6272.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["I8244"] then
	files {
		MAME_DIR .. "src/devices/video/i8244.cpp",
		MAME_DIR .. "src/devices/video/i8244.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["I82730"] then
	files {
		MAME_DIR .. "src/devices/video/i82730.cpp",
		MAME_DIR .. "src/devices/video/i82730.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["I8275"] then
	files {
		MAME_DIR .. "src/devices/video/i8275.cpp",
		MAME_DIR .. "src/devices/video/i8275.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["IMS_CVC"] then
	files {
		MAME_DIR .. "src/devices/video/ims_cvc.cpp",
		MAME_DIR .. "src/devices/video/ims_cvc.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["JANGOU_BLITTER"] then
	files {
		MAME_DIR .. "src/devices/video/jangou_blitter.cpp",
		MAME_DIR .. "src/devices/video/jangou_blitter.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["K051316"] then
	files {
		MAME_DIR .. "src/devices/video/k051316.cpp",
		MAME_DIR .. "src/devices/video/k051316.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["K053936"] then
	files {
		MAME_DIR .. "src/devices/video/k053936.cpp",
		MAME_DIR .. "src/devices/video/k053936.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["KY3211_KY10510"] then
	files {
		MAME_DIR .. "src/devices/video/ky3211_ky10510.cpp",
		MAME_DIR .. "src/devices/video/ky3211_ky10510.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["LC7580"] then
	files {
		MAME_DIR .. "src/devices/video/lc7580.cpp",
		MAME_DIR .. "src/devices/video/lc7580.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["LC7985"] then
	files {
		MAME_DIR .. "src/devices/video/lc7985.cpp",
		MAME_DIR .. "src/devices/video/lc7985.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["M50458"] then
	files {
		MAME_DIR .. "src/devices/video/m50458.cpp",
		MAME_DIR .. "src/devices/video/m50458.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if VIDEOS["MB86292"] then
	files {
		MAME_DIR .. "src/devices/video/mb86292.cpp",
		MAME_DIR .. "src/devices/video/mb86292.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if VIDEOS["MB88303"] then
	files {
		MAME_DIR .. "src/devices/video/mb88303.cpp",
		MAME_DIR .. "src/devices/video/mb88303.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MB90082"] then
	files {
		MAME_DIR .. "src/devices/video/mb90082.cpp",
		MAME_DIR .. "src/devices/video/mb90082.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MB_VCU"] then
	files {
		MAME_DIR .. "src/devices/video/mb_vcu.cpp",
		MAME_DIR .. "src/devices/video/mb_vcu.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MC68328LCD"] then
	files {
		MAME_DIR .. "src/devices/video/mc68328lcd.cpp",
		MAME_DIR .. "src/devices/video/mc68328lcd.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/mc6845.h,VIDEOS["MC6845"] = true
--------------------------------------------------

if VIDEOS["MC6845"] then
	files {
		MAME_DIR .. "src/devices/video/mc6845.cpp",
		MAME_DIR .. "src/devices/video/mc6845.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MC6847"] then
	files {
		MAME_DIR .. "src/devices/video/mc6847.cpp",
		MAME_DIR .. "src/devices/video/mc6847.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MD4330B"] then
	files {
		MAME_DIR .. "src/devices/video/md4330b.cpp",
		MAME_DIR .. "src/devices/video/md4330b.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MM5445"] then
	files {
		MAME_DIR .. "src/devices/video/mm5445.cpp",
		MAME_DIR .. "src/devices/video/mm5445.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MN1252"] then
	files {
		MAME_DIR .. "src/devices/video/mn1252.cpp",
		MAME_DIR .. "src/devices/video/mn1252.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MOS6566"] then
	files {
		MAME_DIR .. "src/devices/video/mos6566.cpp",
		MAME_DIR .. "src/devices/video/mos6566.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MOS8563"] then
	files {
		MAME_DIR .. "src/devices/video/mos8563.cpp",
		MAME_DIR .. "src/devices/video/mos8563.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MR9735"] then
	files {
		MAME_DIR .. "src/devices/video/mr9735.cpp",
		MAME_DIR .. "src/devices/video/mr9735.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MSM6222B"] then
	files {
		MAME_DIR .. "src/devices/video/msm6222b.cpp",
		MAME_DIR .. "src/devices/video/msm6222b.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["MSM6255"] then
	files {
		MAME_DIR .. "src/devices/video/msm6255.cpp",
		MAME_DIR .. "src/devices/video/msm6255.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["NT7534"] then
	files {
		MAME_DIR .. "src/devices/video/nt7534.cpp",
		MAME_DIR .. "src/devices/video/nt7534.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga.cpp",
		MAME_DIR .. "src/devices/video/pc_vga.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_ALLIANCE"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_alliance.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_alliance.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_ATI"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_ati.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_ati.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["IBM8514A"] then
	files {
		MAME_DIR .. "src/devices/video/ibm8514a.cpp",
		MAME_DIR .. "src/devices/video/ibm8514a.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ATI_MACH8"] then
	files {
		MAME_DIR .. "src/devices/video/ati_mach8.cpp",
		MAME_DIR .. "src/devices/video/ati_mach8.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ATI_MACH32"] then
	files {
		MAME_DIR .. "src/devices/video/ati_mach32.cpp",
		MAME_DIR .. "src/devices/video/ati_mach32.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_CHIPS"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_chips.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_chips.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_CIRRUS"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_cirrus.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_cirrus.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_MATROX"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_matrox.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_matrox.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_MEDIAGX"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_mediagx.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_mediagx.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_NVIDIA"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_nvidia.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_nvidia.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_OAK"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_oak.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_oak.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_PARADISE"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_paradise.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_paradise.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_VIDEO7"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_video7.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_video7.h",
	}
end


--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["WD90C26"] then
	files {
		MAME_DIR .. "src/devices/video/wd90c26.cpp",
		MAME_DIR .. "src/devices/video/wd90c26.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_S3"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_s3.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_s3.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["S3VIRGE"] then
	files {
		MAME_DIR .. "src/devices/video/s3virge.cpp",
		MAME_DIR .. "src/devices/video/s3virge.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_SIS"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_sis.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_sis.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_TRIDENT"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_trident.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_trident.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_VGA_TSENG"] then
	files {
		MAME_DIR .. "src/devices/video/pc_vga_tseng.cpp",
		MAME_DIR .. "src/devices/video/pc_vga_tseng.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PC_XGA"] then
	files {
		MAME_DIR .. "src/devices/video/pc_xga.cpp",
		MAME_DIR .. "src/devices/video/pc_xga.h",
	}
end


--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PCD8544"] then
	files {
		MAME_DIR .. "src/devices/video/pcd8544.cpp",
		MAME_DIR .. "src/devices/video/pcd8544.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PCF2100"] then
	files {
		MAME_DIR .. "src/devices/video/pcf2100.cpp",
		MAME_DIR .. "src/devices/video/pcf2100.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PPU2C0X"] then
	files {
		MAME_DIR .. "src/devices/video/ppu2c0x.cpp",
		MAME_DIR .. "src/devices/video/ppu2c0x.h",
		MAME_DIR .. "src/devices/video/ppu2c0x_vt.cpp",
		MAME_DIR .. "src/devices/video/ppu2c0x_vt.h",
		MAME_DIR .. "src/devices/video/ppu2c0x_sh6578.cpp",
		MAME_DIR .. "src/devices/video/ppu2c0x_sh6578.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["PS2GIF"] then
	files {
		MAME_DIR .. "src/devices/video/ps2gif.cpp",
		MAME_DIR .. "src/devices/video/ps2gif.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["PS2GS"] then
	files {
		MAME_DIR .. "src/devices/video/ps2gs.cpp",
		MAME_DIR .. "src/devices/video/ps2gs.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["PSX"] then
	files {
		MAME_DIR .. "src/devices/video/psx.cpp",
		MAME_DIR .. "src/devices/video/psx.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["PWM_DISPLAY"] then
	files {
		MAME_DIR .. "src/devices/video/pwm.cpp",
		MAME_DIR .. "src/devices/video/pwm.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["RAMDAC"] then
	files {
		MAME_DIR .. "src/devices/video/ramdac.cpp",
		MAME_DIR .. "src/devices/video/ramdac.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if VIDEOS["ROC10937"] then
	files {
		MAME_DIR .. "src/devices/video/roc10937.cpp",
		MAME_DIR .. "src/devices/video/roc10937.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["SAA5050"] then
	files {
		MAME_DIR .. "src/devices/video/saa5050.cpp",
		MAME_DIR .. "src/devices/video/saa5050.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["SAA5240"] then
	files {
		MAME_DIR .. "src/devices/video/saa5240.cpp",
		MAME_DIR .. "src/devices/video/saa5240.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["SAA7110"] then
	files {
		MAME_DIR .. "src/devices/video/saa7110.cpp",
		MAME_DIR .. "src/devices/video/saa7110.h",
	}
end

--------------------------------------------------
--
--@src/devices/video/scn2674.h,VIDEOS["SCN2674"] = true
--------------------------------------------------
if VIDEOS["SCN2674"] then
	files {
		MAME_DIR .. "src/devices/video/scn2674.cpp",
		MAME_DIR .. "src/devices/video/scn2674.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["SDA5708"] then
	files {
		MAME_DIR .. "src/devices/video/sda5708.cpp",
		MAME_DIR .. "src/devices/video/sda5708.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["SED1200"] then
	files {
		MAME_DIR .. "src/devices/video/sed1200.cpp",
		MAME_DIR .. "src/devices/video/sed1200.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["SED1330"] then
	files {
		MAME_DIR .. "src/devices/video/sed1330.cpp",
		MAME_DIR .. "src/devices/video/sed1330.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["SED1356"] then
	files {
		MAME_DIR .. "src/devices/video/sed1356.cpp",
		MAME_DIR .. "src/devices/video/sed1356.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["SED1375"] then
	files {
		MAME_DIR .. "src/devices/video/sed1375.cpp",
		MAME_DIR .. "src/devices/video/sed1375.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["SED1500"] then
	files {
		MAME_DIR .. "src/devices/video/sed1500.cpp",
		MAME_DIR .. "src/devices/video/sed1500.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["SED1520"] then
	files {
		MAME_DIR .. "src/devices/video/sed1520.cpp",
		MAME_DIR .. "src/devices/video/sed1520.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["SN74S262"] then
	files {
		MAME_DIR .. "src/devices/video/sn74s262.cpp",
		MAME_DIR .. "src/devices/video/sn74s262.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["SNES_PPU"] then
	files {
		MAME_DIR .. "src/devices/video/snes_ppu.cpp",
		MAME_DIR .. "src/devices/video/snes_ppu.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ST7626"] then
	files {
		MAME_DIR .. "src/devices/video/st7626.cpp",
		MAME_DIR .. "src/devices/video/st7626.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ST7735"] then
	files {
		MAME_DIR .. "src/devices/video/st7735_lcdc.cpp",
		MAME_DIR .. "src/devices/video/st7735_lcdc.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["T6963C"] then
	files {
		MAME_DIR .. "src/devices/video/t6963c.cpp",
		MAME_DIR .. "src/devices/video/t6963c.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["T6A04"] then
	files {
		MAME_DIR .. "src/devices/video/t6a04.cpp",
		MAME_DIR .. "src/devices/video/t6a04.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["TEA1002"] then
	files {
		MAME_DIR .. "src/devices/video/tea1002.cpp",
		MAME_DIR .. "src/devices/video/tea1002.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["TLC34076"] then
	files {
		MAME_DIR .. "src/devices/video/tlc34076.cpp",
		MAME_DIR .. "src/devices/video/tlc34076.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["TMAP038"] then
	files {
		MAME_DIR .. "src/devices/video/tmap038.cpp",
		MAME_DIR .. "src/devices/video/tmap038.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["TMS34061"] then
	files {
		MAME_DIR .. "src/devices/video/tms34061.cpp",
		MAME_DIR .. "src/devices/video/tms34061.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["TMS3556"] then
	files {
		MAME_DIR .. "src/devices/video/tms3556.cpp",
		MAME_DIR .. "src/devices/video/tms3556.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["TMS9927"] then
	files {
		MAME_DIR .. "src/devices/video/tms9927.cpp",
		MAME_DIR .. "src/devices/video/tms9927.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["TMS9928A"] then
	files {
		MAME_DIR .. "src/devices/video/tms9928a.cpp",
		MAME_DIR .. "src/devices/video/tms9928a.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["UPD3301"] then
	files {
		MAME_DIR .. "src/devices/video/upd3301.cpp",
		MAME_DIR .. "src/devices/video/upd3301.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["UPD7220"] then
	files {
		MAME_DIR .. "src/devices/video/upd7220.cpp",
		MAME_DIR .. "src/devices/video/upd7220.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["UPD7227"] then
	files {
		MAME_DIR .. "src/devices/video/upd7227.cpp",
		MAME_DIR .. "src/devices/video/upd7227.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["UPD72120"] then
	files {
		MAME_DIR .. "src/devices/video/upd72120.cpp",
		MAME_DIR .. "src/devices/video/upd72120.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["V9938"] then
	files {
		MAME_DIR .. "src/devices/video/v9938.cpp",
		MAME_DIR .. "src/devices/video/v9938.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["VOODOO"] then
	files {
		MAME_DIR .. "src/devices/video/voodoo.cpp",
		MAME_DIR .. "src/devices/video/voodoo.h",
		MAME_DIR .. "src/devices/video/voodoo_2.cpp",
		MAME_DIR .. "src/devices/video/voodoo_2.h",
		MAME_DIR .. "src/devices/video/voodoo_banshee.cpp",
		MAME_DIR .. "src/devices/video/voodoo_banshee.h",
		MAME_DIR .. "src/devices/video/voodoo_regs.h",
		MAME_DIR .. "src/devices/video/voodoo_render.cpp",
		MAME_DIR .. "src/devices/video/voodoo_render.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["VOODOO_PCI"] then
	files {
		MAME_DIR .. "src/devices/video/voodoo_pci.cpp",
		MAME_DIR .. "src/devices/video/voodoo_pci.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["CRTC_EGA"] then
	files {
		MAME_DIR .. "src/devices/video/crtc_ega.cpp",
		MAME_DIR .. "src/devices/video/crtc_ega.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["I4100"] then
	files {
		MAME_DIR .. "src/devices/video/imagetek_i4100.cpp",
		MAME_DIR .. "src/devices/video/imagetek_i4100.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["TOPCAT"] then
	files {
		MAME_DIR .. "src/devices/video/topcat.cpp",
		MAME_DIR .. "src/devices/video/topcat.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["CATSEYE"] then
	files {
		MAME_DIR .. "src/devices/video/catseye.cpp",
		MAME_DIR .. "src/devices/video/catseye.h",
	}
end


--------------------------------------------------
--
--------------------------------------------------
if VIDEOS["NEREID"] then
	files {
		MAME_DIR .. "src/devices/video/nereid.cpp",
		MAME_DIR .. "src/devices/video/nereid.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["VRENDER0"] then
	files {
		MAME_DIR .. "src/devices/video/vrender0.cpp",
		MAME_DIR .. "src/devices/video/vrender0.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["X1_001"] then
	files {
		MAME_DIR .. "src/devices/video/x1_001.cpp",
		MAME_DIR .. "src/devices/video/x1_001.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["X1_020_DX_101"] then
	files {
		MAME_DIR .. "src/devices/video/x1_020_dx_101.cpp",
		MAME_DIR .. "src/devices/video/x1_020_dx_101.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ZEUS2"] then
	files {
		MAME_DIR .. "src/devices/video/zeus2.cpp",
		MAME_DIR .. "src/devices/video/zeus2.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ZR36060"] then
	files {
		MAME_DIR .. "src/devices/video/zr36060.cpp",
		MAME_DIR .. "src/devices/video/zr36060.h",
	}
end

--------------------------------------------------
--
--------------------------------------------------

if VIDEOS["ZR36110"] then
	files {
		MAME_DIR .. "src/devices/video/zr36110.cpp",
		MAME_DIR .. "src/devices/video/zr36110.h",
	}
end
