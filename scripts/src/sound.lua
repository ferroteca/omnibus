-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   sound.lua
--
--   Rules for building sound cores
--
----------------------------------------------------------------------------

files {
	MAME_DIR .. "src/devices/sound/bbd.cpp",
	MAME_DIR .. "src/devices/sound/bbd.h",
	MAME_DIR .. "src/devices/sound/flt_biquad.cpp",
	MAME_DIR .. "src/devices/sound/flt_biquad.h",
	MAME_DIR .. "src/devices/sound/flt_vol.cpp",
	MAME_DIR .. "src/devices/sound/flt_vol.h",
	MAME_DIR .. "src/devices/sound/flt_rc.cpp",
	MAME_DIR .. "src/devices/sound/flt_rc.h",
	MAME_DIR .. "src/devices/sound/mixer.cpp",
	MAME_DIR .. "src/devices/sound/mixer.h",
	MAME_DIR .. "src/devices/sound/samples.cpp",
	MAME_DIR .. "src/devices/sound/samples.h",
}

---------------------------------------------------
-- DACs
--@src/devices/sound/spkrdev.h,SOUNDS["SPEAKER"] = true
--@src/devices/sound/beep.h,SOUNDS["BEEP"] = true
---------------------------------------------------

if SOUNDS["DAC"] then
	files {
		MAME_DIR .. "src/devices/sound/dac.cpp",
		MAME_DIR .. "src/devices/sound/dac.h",
	}
end

if SOUNDS["DMADAC"] then
	files {
		MAME_DIR .. "src/devices/sound/dmadac.cpp",
		MAME_DIR .. "src/devices/sound/dmadac.h",
	}
end

if SOUNDS["SPEAKER"] then
	files {
		MAME_DIR .. "src/devices/sound/spkrdev.cpp",
		MAME_DIR .. "src/devices/sound/spkrdev.h",
	}
end

if SOUNDS["BEEP"] then
	files {
		MAME_DIR .. "src/devices/sound/beep.cpp",
		MAME_DIR .. "src/devices/sound/beep.h",
	}
end



---------------------------------------------------
-- CD audio
--@src/devices/sound/cdda.h,SOUNDS["CDDA"] = true
---------------------------------------------------

if SOUNDS["CDDA"] then
	files {
		MAME_DIR .. "src/devices/sound/cdda.cpp",
		MAME_DIR .. "src/devices/sound/cdda.h",
	}
end



---------------------------------------------------
-- Discrete component audio
---------------------------------------------------

if SOUNDS["DISCRETE"] then
	files {
		MAME_DIR .. "src/devices/sound/discrete.cpp",
		MAME_DIR .. "src/devices/sound/discrete.h",
		MAME_DIR .. "src/devices/sound/disc_cls.h",
		MAME_DIR .. "src/devices/sound/disc_dev.h",
		MAME_DIR .. "src/devices/sound/disc_dev.hxx",
		MAME_DIR .. "src/devices/sound/disc_flt.h",
		MAME_DIR .. "src/devices/sound/disc_flt.hxx",
		MAME_DIR .. "src/devices/sound/disc_inp.hxx",
		MAME_DIR .. "src/devices/sound/disc_mth.h",
		MAME_DIR .. "src/devices/sound/disc_mth.hxx",
		MAME_DIR .. "src/devices/sound/disc_sys.hxx",
		MAME_DIR .. "src/devices/sound/disc_wav.h",
		MAME_DIR .. "src/devices/sound/disc_wav.hxx",
	}
end

---------------------------------------------------
-- AC97
---------------------------------------------------

if SOUNDS["AC97"] then
	files {
		MAME_DIR .. "src/devices/sound/pci-ac97.cpp",
		MAME_DIR .. "src/devices/sound/pci-ac97.h",
	}
end

---------------------------------------------------
-- SigmaTel STAC9704
---------------------------------------------------

if SOUNDS["AC97_STAC9704"] then
	files {
		MAME_DIR .. "src/devices/sound/ac97_stac9704.cpp",
		MAME_DIR .. "src/devices/sound/ac97_stac9704.h",
	}
end


---------------------------------------------------
-- Apple custom sound chips
---------------------------------------------------

if SOUNDS["ASC"] then
	files {
		MAME_DIR .. "src/devices/sound/asc.cpp",
		MAME_DIR .. "src/devices/sound/asc.h",
	}
end

if SOUNDS["AWACS"] then
	files {
		MAME_DIR .. "src/devices/sound/awacs.cpp",
		MAME_DIR .. "src/devices/sound/awacs.h",
	}
end


---------------------------------------------------
-- Atari custom sound chips
---------------------------------------------------

if SOUNDS["POKEY"] then
	files {
		MAME_DIR .. "src/devices/sound/pokey.cpp",
		MAME_DIR .. "src/devices/sound/pokey.h",
	}
end

if SOUNDS["TIA"] then
	files {
		MAME_DIR .. "src/devices/sound/tiasound.cpp",
		MAME_DIR .. "src/devices/sound/tiasound.h",
		MAME_DIR .. "src/devices/sound/tiaintf.cpp",
		MAME_DIR .. "src/devices/sound/tiaintf.h",
	}
end

---------------------------------------------------
-- Bally Astrocade sound system
---------------------------------------------------

if SOUNDS["ASTROCADE"] then
	files {
		MAME_DIR .. "src/devices/sound/astrocde.cpp",
		MAME_DIR .. "src/devices/sound/astrocde.h",
	}
end



---------------------------------------------------
-- AC97
---------------------------------------------------

if SOUNDS["AC97"] then
	files {
		MAME_DIR .. "src/devices/sound/pci-ac97.cpp",
		MAME_DIR .. "src/devices/sound/pci-ac97.h",
	}
end



---------------------------------------------------
-- CEM 3310 envelope generator chip
---------------------------------------------------

if SOUNDS["CEM3310"] then
	files {
		MAME_DIR .. "src/devices/sound/cem3310.cpp",
		MAME_DIR .. "src/devices/sound/cem3310.h",
	}
end

---------------------------------------------------
-- CEM 3320 voltage-controlled filter chip
---------------------------------------------------

if SOUNDS["CEM3320"] then
	files {
		MAME_DIR .. "src/devices/sound/cem3320.cpp",
		MAME_DIR .. "src/devices/sound/cem3320.h",
	}
end

---------------------------------------------------
-- CEM 3340 voltage-controlled oscillator chip
---------------------------------------------------

if SOUNDS["CEM3340"] then
	files {
		MAME_DIR .. "src/devices/sound/cem3340.cpp",
		MAME_DIR .. "src/devices/sound/cem3340.h",
	}
end

---------------------------------------------------
-- CEM 3360 dual voltage-controlled amplifier chip
---------------------------------------------------

if SOUNDS["CEM3360"] then
	files {
		MAME_DIR .. "src/devices/sound/cem3360.cpp",
		MAME_DIR .. "src/devices/sound/cem3360.h",
	}
end

---------------------------------------------------
-- CEM 3394 analog synthesizer chip
---------------------------------------------------

if SOUNDS["CEM3394"] then
	files {
		MAME_DIR .. "src/devices/sound/cem3394.cpp",
		MAME_DIR .. "src/devices/sound/cem3394.h",
	}
end



---------------------------------------------------
-- Creative Labs CT1741 SB16 DSP
---------------------------------------------------

if SOUNDS["CT1741"] then
	files {
		MAME_DIR .. "src/devices/sound/ct1741.cpp",
		MAME_DIR .. "src/devices/sound/ct1741.h",
	}
end

---------------------------------------------------
-- Creative Labs CT1745 SB16 Mixer
---------------------------------------------------

if SOUNDS["CT1745"] then
	files {
		MAME_DIR .. "src/devices/sound/ct1745.cpp",
		MAME_DIR .. "src/devices/sound/ct1745.h",
	}
end



---------------------------------------------------
-- Creative Labs SB0400 Audigy2 Value
---------------------------------------------------

if SOUNDS["SB0400"] then
	files {
		MAME_DIR .. "src/devices/sound/sb0400.cpp",
		MAME_DIR .. "src/devices/sound/sb0400.h",
	}
end


--------------------------------------------------
-- Creative Labs Ensonic AudioPCI97 ES1373
--------------------------------------------------

if SOUNDS["ES1373"] then
	files {
		MAME_DIR .. "src/devices/sound/es1373.cpp",
		MAME_DIR .. "src/devices/sound/es1373.h",
	}
end

---------------------------------------------------
-- Ensoniq 5503 (Apple IIgs)
---------------------------------------------------

if SOUNDS["ES5503"] then
	files {
		MAME_DIR .. "src/devices/sound/es5503.cpp",
		MAME_DIR .. "src/devices/sound/es5503.h",
	}
end


---------------------------------------------------
-- Ensoniq 5505/5506
---------------------------------------------------

if SOUNDS["ES5505"]~=null or SOUNDS["ES5506"] then
	files {
		MAME_DIR .. "src/devices/sound/es5506.cpp",
		MAME_DIR .. "src/devices/sound/es5506.h",
	}
end


---------------------------------------------------
-- Ensoniq "pump" device, interfaces 5505/5506 with 5510
---------------------------------------------------

if SOUNDS["ESQPUMP"] then
	files {
		MAME_DIR .. "src/devices/sound/esqpump.cpp",
		MAME_DIR .. "src/devices/sound/esqpump.h",
	}
end


---------------------------------------------------
-- Data East custom sound chips
---------------------------------------------------

if SOUNDS["BSMT2000"] then
	files {
		MAME_DIR .. "src/devices/sound/bsmt2000.cpp",
		MAME_DIR .. "src/devices/sound/bsmt2000.h",
	}
end



---------------------------------------------------
-- Excellent Systems ADPCM sound chip
---------------------------------------------------

if SOUNDS["ES8712"] then
	files {
		MAME_DIR .. "src/devices/sound/es8712.cpp",
		MAME_DIR .. "src/devices/sound/es8712.h",
	}
end



---------------------------------------------------
-- Gaelco custom sound chips
---------------------------------------------------

if SOUNDS["GAELCO_CG1V"]~=null or SOUNDS["GAELCO_GAE1"] then
	files {
		MAME_DIR .. "src/devices/sound/gaelco.cpp",
		MAME_DIR .. "src/devices/sound/gaelco.h",
	}
end


---------------------------------------------------
-- RCA CDP1863
---------------------------------------------------

if SOUNDS["CDP1863"] then
	files {
		MAME_DIR .. "src/devices/sound/cdp1863.cpp",
		MAME_DIR .. "src/devices/sound/cdp1863.h",
	}
end



---------------------------------------------------
-- RCA CDP1864
---------------------------------------------------

if SOUNDS["CDP1864"] then
	files {
		MAME_DIR .. "src/devices/sound/cdp1864.cpp",
		MAME_DIR .. "src/devices/sound/cdp1864.h",
	}
end



---------------------------------------------------
-- RCA CDP1869
---------------------------------------------------

if SOUNDS["CDP1869"] then
	files {
		MAME_DIR .. "src/devices/sound/cdp1869.cpp",
		MAME_DIR .. "src/devices/sound/cdp1869.h",
	}
end



---------------------------------------------------
-- GI AY-8910
--@src/devices/sound/ay8910.h,SOUNDS["AY8910"] = true
---------------------------------------------------

if SOUNDS["AY8910"] then
	files {
		MAME_DIR .. "src/devices/sound/ay8910.cpp",
		MAME_DIR .. "src/devices/sound/ay8910.h",
	}
end



---------------------------------------------------
-- Harris HC55516 CVSD
---------------------------------------------------

if SOUNDS["HC55516"] then
	files {
		MAME_DIR .. "src/devices/sound/hc55516.cpp",
		MAME_DIR .. "src/devices/sound/hc55516.h",
	}
end



---------------------------------------------------
-- Hudsonsoft HuC6230 SoundBox
---------------------------------------------------

if SOUNDS["HUC6230"] then
	files {
		MAME_DIR .. "src/devices/sound/huc6230.cpp",
		MAME_DIR .. "src/devices/sound/huc6230.h",
	}
end



---------------------------------------------------
-- Hudsonsoft C6280 sound chip
---------------------------------------------------

if SOUNDS["C6280"] then
	files {
		MAME_DIR .. "src/devices/sound/c6280.cpp",
		MAME_DIR .. "src/devices/sound/c6280.h",
	}
end



---------------------------------------------------
-- ICS2115 sound chip
---------------------------------------------------

if SOUNDS["ICS2115"] then
	files {
		MAME_DIR .. "src/devices/sound/ics2115.cpp",
		MAME_DIR .. "src/devices/sound/ics2115.h",
	}
end



---------------------------------------------------
-- Imagetek I5000 sound
---------------------------------------------------

if SOUNDS["I5000_SND"] then
	files {
		MAME_DIR .. "src/devices/sound/i5000.cpp",
		MAME_DIR .. "src/devices/sound/i5000.h",
	}
end



---------------------------------------------------
-- Irem custom sound chips
---------------------------------------------------

if SOUNDS["IREMGA20"] then
	files {
		MAME_DIR .. "src/devices/sound/iremga20.cpp",
		MAME_DIR .. "src/devices/sound/iremga20.h",
	}
end



---------------------------------------------------
-- Konami custom sound chips
---------------------------------------------------

if SOUNDS["K005289"] then
	files {
		MAME_DIR .. "src/devices/sound/k005289.cpp",
		MAME_DIR .. "src/devices/sound/k005289.h",
	}
end

if SOUNDS["K007232"] then
	files {
		MAME_DIR .. "src/devices/sound/k007232.cpp",
		MAME_DIR .. "src/devices/sound/k007232.h",
	}
end

if SOUNDS["K051649"] then
	files {
		MAME_DIR .. "src/devices/sound/k051649.cpp",
		MAME_DIR .. "src/devices/sound/k051649.h",
	}
end

if SOUNDS["K053260"] then
	files {
		MAME_DIR .. "src/devices/sound/k053260.cpp",
		MAME_DIR .. "src/devices/sound/k053260.h",
	}
end

if SOUNDS["K054321"] then
	files {
		MAME_DIR .. "src/devices/sound/k054321.cpp",
		MAME_DIR .. "src/devices/sound/k054321.h",
	}
end

if SOUNDS["K054539"] then
	files {
		MAME_DIR .. "src/devices/sound/k054539.cpp",
		MAME_DIR .. "src/devices/sound/k054539.h",
	}
end

if SOUNDS["K056800"] then
	files {
		MAME_DIR .. "src/devices/sound/k056800.cpp",
		MAME_DIR .. "src/devices/sound/k056800.h",
	}
end

---------------------------------------------------
-- L4003
---------------------------------------------------

if SOUNDS["L4003"] then
	files {
		MAME_DIR .. "src/devices/sound/l4003.cpp",
		MAME_DIR .. "src/devices/sound/l4003.h",
	}
end

---------------------------------------------------
-- L7A1045 L6028 DSP-A
---------------------------------------------------

if SOUNDS["L7A1045"] then
	files {
		MAME_DIR .. "src/devices/sound/l7a1045_l6028_dsp_a.cpp",
		MAME_DIR .. "src/devices/sound/l7a1045_l6028_dsp_a.h",
	}
end


---------------------------------------------------
-- LMC1992 mixer chip
---------------------------------------------------

if SOUNDS["LMC1992"] then
	files {
		MAME_DIR .. "src/devices/sound/lmc1992.cpp",
		MAME_DIR .. "src/devices/sound/lmc1992.h",
	}
end



---------------------------------------------------
-- MAS 3507D MPEG 1/2 Layer 2/3 Audio Decoder
---------------------------------------------------

if SOUNDS["MAS3507D"] then
	files {
		MAME_DIR .. "src/devices/sound/mas3507d.cpp",
		MAME_DIR .. "src/devices/sound/mas3507d.h",
	}
end



---------------------------------------------------
-- Fujitsu MB87077 volume controller
---------------------------------------------------

if SOUNDS["MB87077"] then
	files {
		MAME_DIR .. "src/devices/sound/mb87077.cpp",
		MAME_DIR .. "src/devices/sound/mb87077.h",
	}
end



---------------------------------------------------
-- Micronas DAC 3550A Stereo Audio DAC
---------------------------------------------------

if SOUNDS["DAC3350A"] then
	files {
		MAME_DIR .. "src/devices/sound/dac3350a.cpp",
		MAME_DIR .. "src/devices/sound/dac3350a.h",
	}
end



---------------------------------------------------
-- MEA8000 Voice Synthesizer
---------------------------------------------------

if SOUNDS["MEA8000"] then
	files {
		MAME_DIR .. "src/devices/sound/mea8000.cpp",
		MAME_DIR .. "src/devices/sound/mea8000.h",
	}
end



---------------------------------------------------
-- MOS 6560VIC
---------------------------------------------------

if SOUNDS["MOS656X"] then
	files {
		MAME_DIR .. "src/devices/sound/mos6560.cpp",
		MAME_DIR .. "src/devices/sound/mos6560.h",
	}
end



---------------------------------------------------
-- MOS 7360 TED
---------------------------------------------------

if SOUNDS["MOS7360"] then
	files {
		MAME_DIR .. "src/devices/sound/mos7360.cpp",
		MAME_DIR .. "src/devices/sound/mos7360.h",
	}
end



---------------------------------------------------
-- Namco custom sound chips
---------------------------------------------------

if SOUNDS["NAMCO"]~=null or SOUNDS["NAMCO_15XX"]~=null or SOUNDS["NAMCO_CUS30"] then
	files {
		MAME_DIR .. "src/devices/sound/namco.cpp",
		MAME_DIR .. "src/devices/sound/namco.h",
	}
end

if SOUNDS["NAMCO_163"] then
	files {
		MAME_DIR .. "src/devices/sound/namco_163.cpp",
		MAME_DIR .. "src/devices/sound/namco_163.h",
	}
end

if SOUNDS["NAMCO_63701X"] then
	files {
		MAME_DIR .. "src/devices/sound/n63701x.cpp",
		MAME_DIR .. "src/devices/sound/n63701x.h",
	}
end

if SOUNDS["C140"] then
	files {
		MAME_DIR .. "src/devices/sound/c140.cpp",
		MAME_DIR .. "src/devices/sound/c140.h",
	}
end

if SOUNDS["C352"] then
	files {
		MAME_DIR .. "src/devices/sound/c352.cpp",
		MAME_DIR .. "src/devices/sound/c352.h",
	}
end



---------------------------------------------------
-- National Semiconductor Digitalker
---------------------------------------------------

if SOUNDS["DIGITALKER"] then
	files {
		MAME_DIR .. "src/devices/sound/digitalk.cpp",
		MAME_DIR .. "src/devices/sound/digitalk.h",
	}
end



---------------------------------------------------
-- Nintendo custom sound chips
---------------------------------------------------

if SOUNDS["NES_APU"] then
	files {
		MAME_DIR .. "src/devices/sound/nes_apu.cpp",
		MAME_DIR .. "src/devices/sound/nes_apu.h",
		MAME_DIR .. "src/devices/sound/nes_defs.h",
		MAME_DIR .. "src/devices/sound/nes_apu_vt.cpp",
		MAME_DIR .. "src/devices/sound/nes_apu_vt.h",
	}
end



---------------------------------------------------
-- NEC uPD7759 ADPCM sample player
---------------------------------------------------

if SOUNDS["UPD7759"] then
	files {
		MAME_DIR .. "src/devices/sound/upd7759.cpp",
		MAME_DIR .. "src/devices/sound/upd7759.h",
		MAME_DIR .. "src/devices/sound/315-5641.cpp",
		MAME_DIR .. "src/devices/sound/315-5641.h",
	}
end



---------------------------------------------------
-- IMA ADPCM sample player
---------------------------------------------------

if SOUNDS["IMAADPCM"] then
	files {
		MAME_DIR .. "src/devices/sound/imaadpcm.cpp",
		MAME_DIR .. "src/devices/sound/imaadpcm.h",
	}
end



---------------------------------------------------
-- OKI ADPCM sample players
---------------------------------------------------

if SOUNDS["OKIM6258"]~=null or SOUNDS["OKIM6295"]~=null or SOUNDS["OKIM6588"]~=null or SOUNDS["OKIM9810"]~=null or SOUNDS["I5000_SND"]~=null or SOUNDS["OKIADPCM"] then
	files {
		MAME_DIR .. "src/devices/sound/okiadpcm.cpp",
		MAME_DIR .. "src/devices/sound/okiadpcm.h",
	}
end

if SOUNDS["MSM5205"]~=null or SOUNDS["MSM6585"] then
	files {
		MAME_DIR .. "src/devices/sound/msm5205.cpp",
		MAME_DIR .. "src/devices/sound/msm5205.h",
	}
end

if SOUNDS["MSM5232"] then
	files {
		MAME_DIR .. "src/devices/sound/msm5232.cpp",
		MAME_DIR .. "src/devices/sound/msm5232.h",
	}
end

if SOUNDS["OKIM6376"] then
	files {
		MAME_DIR .. "src/devices/sound/okim6376.cpp",
		MAME_DIR .. "src/devices/sound/okim6376.h",
	}
end

if SOUNDS["OKIM6295"] then
	files {
		MAME_DIR .. "src/devices/sound/okim6295.cpp",
		MAME_DIR .. "src/devices/sound/okim6295.h",
	}
end

if SOUNDS["OKIM6258"] then
	files {
		MAME_DIR .. "src/devices/sound/okim6258.cpp",
		MAME_DIR .. "src/devices/sound/okim6258.h",
	}
end

if SOUNDS["OKIM6588"] then
	files {
		MAME_DIR .. "src/devices/sound/okim6588.cpp",
		MAME_DIR .. "src/devices/sound/okim6588.h",
	}
end

if SOUNDS["OKIM9810"] then
	files {
		MAME_DIR .. "src/devices/sound/okim9810.cpp",
		MAME_DIR .. "src/devices/sound/okim9810.h",
	}
end



---------------------------------------------------
-- Philips SAA1099
---------------------------------------------------

if SOUNDS["SAA1099"] then
	files {
		MAME_DIR .. "src/devices/sound/saa1099.cpp",
		MAME_DIR .. "src/devices/sound/saa1099.h",
	}
end



---------------------------------------------------
-- AdMOS QS1000
---------------------------------------------------

if SOUNDS["QS1000"] then
	files {
		MAME_DIR .. "src/devices/sound/qs1000.cpp",
		MAME_DIR .. "src/devices/sound/qs1000.h",
	}
end



---------------------------------------------------
-- QSound sample player
---------------------------------------------------

if SOUNDS["QSOUND"] then
	files {
		MAME_DIR .. "src/devices/sound/qsound.cpp",
		MAME_DIR .. "src/devices/sound/qsound.h",
		MAME_DIR .. "src/devices/sound/qsoundhle.cpp",
		MAME_DIR .. "src/devices/sound/qsoundhle.h",
	}
end



---------------------------------------------------
-- Ricoh sample players
---------------------------------------------------

if SOUNDS["RF5C68"] then
	files {
		MAME_DIR .. "src/devices/sound/rf5c68.cpp",
		MAME_DIR .. "src/devices/sound/rf5c68.h",
	}
end

if SOUNDS["RF5C400"] then
	files {
		MAME_DIR .. "src/devices/sound/rf5c400.cpp",
		MAME_DIR .. "src/devices/sound/rf5c400.h",
	}
end



---------------------------------------------------
-- Sega custom sound chips
---------------------------------------------------

if SOUNDS["SEGAPCM"] then
	files {
		MAME_DIR .. "src/devices/sound/segapcm.cpp",
		MAME_DIR .. "src/devices/sound/segapcm.h",
	}
end

if SOUNDS["SCSP"] then
	files {
		MAME_DIR .. "src/devices/sound/scsp.cpp",
		MAME_DIR .. "src/devices/sound/scsp.h",
		MAME_DIR .. "src/devices/sound/scspdsp.cpp",
		MAME_DIR .. "src/devices/sound/scspdsp.h",
	}
end

if SOUNDS["AICA"] then
	files {
		MAME_DIR .. "src/devices/sound/aica.cpp",
		MAME_DIR .. "src/devices/sound/aica.h",
		MAME_DIR .. "src/devices/sound/aicadsp.cpp",
		MAME_DIR .. "src/devices/sound/aicadsp.h",
	}
end

---------------------------------------------------
-- Seta custom sound chips
---------------------------------------------------

if SOUNDS["ST0016"] then
	files {
		MAME_DIR .. "src/devices/sound/st0016.cpp",
		MAME_DIR .. "src/devices/sound/st0016.h",
	}
end

if SOUNDS["SETAPCM"] then
	files {
		MAME_DIR .. "src/devices/sound/setapcm.cpp",
		MAME_DIR .. "src/devices/sound/setapcm.h",
	}
end

if SOUNDS["X1_010"] then
	files {
		MAME_DIR .. "src/devices/sound/x1_010.cpp",
		MAME_DIR .. "src/devices/sound/x1_010.h",
	}
end



---------------------------------------------------
-- SID custom sound chips
---------------------------------------------------

if SOUNDS["SID6581"]~=null or SOUNDS["SID8580"] then
	files {
		MAME_DIR .. "src/devices/sound/mos6581.cpp",
		MAME_DIR .. "src/devices/sound/mos6581.h",
		MAME_DIR .. "src/devices/sound/sid.cpp",
		MAME_DIR .. "src/devices/sound/sid.h",
		MAME_DIR .. "src/devices/sound/sidenvel.cpp",
		MAME_DIR .. "src/devices/sound/sidenvel.h",
		MAME_DIR .. "src/devices/sound/sidvoice.cpp",
		MAME_DIR .. "src/devices/sound/sidvoice.h",
		MAME_DIR .. "src/devices/sound/side6581.h",
		MAME_DIR .. "src/devices/sound/sidw6581.h",
		MAME_DIR .. "src/devices/sound/sidw8580.h",
	}
end


---------------------------------------------------
-- SNK(?) custom stereo sn76489a clone
---------------------------------------------------

if SOUNDS["T6W28"] then
	files {
		MAME_DIR .. "src/devices/sound/t6w28.cpp",
		MAME_DIR .. "src/devices/sound/t6w28.h",
	}
end



---------------------------------------------------
-- SNK custom wave generator
---------------------------------------------------

if SOUNDS["SNKWAVE"] then
	files {
		MAME_DIR .. "src/devices/sound/snkwave.cpp",
		MAME_DIR .. "src/devices/sound/snkwave.h",
	}
end



---------------------------------------------------
-- Sony custom sound chips
---------------------------------------------------

if SOUNDS["SPU"] then
	files {
		MAME_DIR .. "src/devices/sound/spu.cpp",
		MAME_DIR .. "src/devices/sound/spu.h",
		MAME_DIR .. "src/devices/sound/spu_tables.cpp",
		MAME_DIR .. "src/devices/sound/spureverb.cpp",
		MAME_DIR .. "src/devices/sound/spureverb.h",
	}
end


---------------------------------------------------
-- SP0256 speech synthesizer
---------------------------------------------------

if SOUNDS["SP0256"] then
	files {
		MAME_DIR .. "src/devices/sound/sp0256.cpp",
		MAME_DIR .. "src/devices/sound/sp0256.h",
	}
end



---------------------------------------------------
-- SP0250 speech synthesizer
---------------------------------------------------

if SOUNDS["SP0250"] then
	files {
		MAME_DIR .. "src/devices/sound/sp0250.cpp",
		MAME_DIR .. "src/devices/sound/sp0250.h",
	}
end


---------------------------------------------------
-- ST-Techno custom sound chip
---------------------------------------------------

if SOUNDS["STT_SA1"] then
	files {
		MAME_DIR .. "src/devices/sound/stt_sa1.cpp",
		MAME_DIR .. "src/devices/sound/stt_sa1.h",
	}
end


---------------------------------------------------
-- S14001A speech synthesizer
---------------------------------------------------

if SOUNDS["S14001A"] then
	files {
		MAME_DIR .. "src/devices/sound/s14001a.cpp",
		MAME_DIR .. "src/devices/sound/s14001a.h",
	}
end



---------------------------------------------------
-- Texas Instruments SN76477 analog chip
---------------------------------------------------

if SOUNDS["SN76477"] then
	files {
		MAME_DIR .. "src/devices/sound/sn76477.cpp",
		MAME_DIR .. "src/devices/sound/sn76477.h",
	}
end



---------------------------------------------------
-- Texas Instruments SN76496
---------------------------------------------------

if SOUNDS["SN76496"] then
	files {
		MAME_DIR .. "src/devices/sound/sn76496.cpp",
		MAME_DIR .. "src/devices/sound/sn76496.h",
	}
end



---------------------------------------------------
-- Silicon Systems SSI-263A HLE
---------------------------------------------------

if SOUNDS["SSI263HLE"] then
	files {
		MAME_DIR .. "src/devices/sound/ssi263hle.cpp",
		MAME_DIR .. "src/devices/sound/ssi263hle.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS36xx doorbell chime
---------------------------------------------------

if SOUNDS["TMS36XX"] then
	files {
		MAME_DIR .. "src/devices/sound/tms36xx.cpp",
		MAME_DIR .. "src/devices/sound/tms36xx.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS3615 Octave Multiple Tone Synthesizer
---------------------------------------------------

if SOUNDS["TMS3615"] then
	files {
		MAME_DIR .. "src/devices/sound/tms3615.cpp",
		MAME_DIR .. "src/devices/sound/tms3615.h",
	}
end



---------------------------------------------------
-- Texas Instruments TMS3631
---------------------------------------------------

if SOUNDS["TMS3631"] then
	files {
		MAME_DIR .. "src/devices/sound/tms3631.cpp",
		MAME_DIR .. "src/devices/sound/tms3631.h",
	}
end

---------------------------------------------------
-- Texas Instruments TMS5100-series speech synthesizers
---------------------------------------------------

if SOUNDS["TMS5110"] then
	files {
		MAME_DIR .. "src/devices/sound/tms5110.cpp",
		MAME_DIR .. "src/devices/sound/tms5110.h",
		MAME_DIR .. "src/devices/sound/tms5110r.hxx",
	}
end

---------------------------------------------------
-- Texas Instruments TMS5200-series speech synthesizers
---------------------------------------------------
if SOUNDS["TMS5220"] then
	files {
		MAME_DIR .. "src/devices/sound/tms5220.cpp",
		MAME_DIR .. "src/devices/sound/tms5220.h",
		MAME_DIR .. "src/devices/sound/tms5110r.hxx",
	}
end


---------------------------------------------------
-- Toshiba T6721A voice synthesizer
---------------------------------------------------

if SOUNDS["T6721A"] then
	files {
		MAME_DIR .. "src/devices/sound/t6721a.cpp",
		MAME_DIR .. "src/devices/sound/t6721a.h",
	}
end



---------------------------------------------------
-- Toshiba TC8830F sample player/recorder
---------------------------------------------------

if SOUNDS["TC8830F"] then
	files {
		MAME_DIR .. "src/devices/sound/tc8830f.cpp",
		MAME_DIR .. "src/devices/sound/tc8830f.h",
	}
end


---------------------------------------------------
-- NEC uPD7752
---------------------------------------------------

if SOUNDS["UPD7752"] then
	files {
		MAME_DIR .. "src/devices/sound/upd7752.cpp",
		MAME_DIR .. "src/devices/sound/upd7752.h",
	}
end

--------------------------------------------------
-- Virtual analog envelope generators (EGs)
--------------------------------------------------

if SOUNDS["VA_EG"] then
	files {
		MAME_DIR .. "src/devices/sound/va_eg.cpp",
		MAME_DIR .. "src/devices/sound/va_eg.h",
	}
end

--------------------------------------------------
-- Virtual analog operations
--------------------------------------------------

if SOUNDS["VA_OPS"] then
	files {
		MAME_DIR .. "src/devices/sound/va_ops.cpp",
		MAME_DIR .. "src/devices/sound/va_ops.h",
	}
end

--------------------------------------------------
-- Virtual analog voltage-controlled amplifiers (VCAs)
--------------------------------------------------

if SOUNDS["VA_VCA"] then
	files {
		MAME_DIR .. "src/devices/sound/va_vca.cpp",
		MAME_DIR .. "src/devices/sound/va_vca.h",
	}
end

--------------------------------------------------
-- Virtual analog voltage-controlled oscillator (VCO)
--------------------------------------------------

if SOUNDS["VA_VCO"] then
	files {
		MAME_DIR .. "src/devices/sound/va_vco.cpp",
		MAME_DIR .. "src/devices/sound/va_vco.h",
	}
end

--------------------------------------------------
-- Virtual analog voltage-controlled filters (VCFs)
--------------------------------------------------

if SOUNDS["VA_VCF"] then
	files {
		MAME_DIR .. "src/devices/sound/va_vcf.cpp",
		MAME_DIR .. "src/devices/sound/va_vcf.h",
	}
end

---------------------------------------------------
-- VLM5030 speech synthesizer
---------------------------------------------------

if SOUNDS["VLM5030"] then
	files {
		MAME_DIR .. "src/devices/sound/vlm5030.cpp",
		MAME_DIR .. "src/devices/sound/vlm5030.h",
		MAME_DIR .. "src/devices/sound/tms5110r.hxx",
	}
end

---------------------------------------------------
-- Votrax SC-01[-A] speech synthesizer
--@src/devices/sound/votrax.h,SOUNDS["VOTRAX_SC01"] = true
--@src/devices/sound/votrax.h,SOUNDS["VOTRAX_SC01A"] = true
---------------------------------------------------

if SOUNDS["VOTRAX_SC01"]~=null or SOUNDS["VOTRAX_SC01A"] then
	files {
		MAME_DIR .. "src/devices/sound/votrax.cpp",
		MAME_DIR .. "src/devices/sound/votrax.h",
	}
end



---------------------------------------------------
-- VRender0 custom sound chip
---------------------------------------------------

if SOUNDS["VRENDER0"] then
	files {
		MAME_DIR .. "src/devices/sound/vrender0.cpp",
		MAME_DIR .. "src/devices/sound/vrender0.h",
	}
end



---------------------------------------------------
-- Yamaha FM synthesizers
---------------------------------------------------

if SOUNDS["YM2154"] then
	files {
		MAME_DIR .. "src/devices/sound/ym2154.cpp",
		MAME_DIR .. "src/devices/sound/ym2154.h",
	}
end

if SOUNDS["YM2151"]~=null or SOUNDS["YM2164"] then
	files {
		MAME_DIR .. "src/devices/sound/ymopm.cpp",
		MAME_DIR .. "src/devices/sound/ymopm.h",
	}
end

if SOUNDS["YM2414"] then
	files {
		MAME_DIR .. "src/devices/sound/ymopz.cpp",
		MAME_DIR .. "src/devices/sound/ymopz.h",
	}
end

if SOUNDS["YM3806"] then
	files {
		MAME_DIR .. "src/devices/sound/ymopq.cpp",
		MAME_DIR .. "src/devices/sound/ymopq.h",
	}
end

if SOUNDS["YM2203"]~=null or SOUNDS["YM2608"]~=null or SOUNDS["YM2610"]~=null or SOUNDS["YM2610B"]~=null or SOUNDS["YM2612"]~=null or SOUNDS["YM3438"] then
	files {
		MAME_DIR .. "src/devices/sound/ay8910.cpp",
		MAME_DIR .. "src/devices/sound/ay8910.h",
		MAME_DIR .. "src/devices/sound/ymopn.cpp",
		MAME_DIR .. "src/devices/sound/ymopn.h",
	}
end

if SOUNDS["YM3526"]~=null or SOUNDS["Y8950"]~=null or SOUNDS["YM3812"]~=null or SOUNDS["YMF262"]~=null or SOUNDS["YMF278B"]~=null or SOUNDS["YM2413"]~=null or SOUNDS["YM2423"]~=null or SOUNDS["YMF281"]~=null or SOUNDS["DS1001"] then
	files {
		MAME_DIR .. "src/devices/sound/ymopl.cpp",
		MAME_DIR .. "src/devices/sound/ymopl.h",
	}
end

if SOUNDS["YMF271"] then
	files {
		MAME_DIR .. "src/devices/sound/ymf271.cpp",
		MAME_DIR .. "src/devices/sound/ymf271.h",
	}
end



---------------------------------------------------
-- Yamaha YMZ280B ADPCM
---------------------------------------------------

if SOUNDS["YMZ280B"] then
	files {
		MAME_DIR .. "src/devices/sound/ymz280b.cpp",
		MAME_DIR .. "src/devices/sound/ymz280b.h",
	}
end

---------------------------------------------------
-- Yamaha YMZ770 AMM
---------------------------------------------------

if SOUNDS["YMZ770"] then
	files {
		MAME_DIR .. "src/devices/sound/ymz770.cpp",
		MAME_DIR .. "src/devices/sound/ymz770.h",
	}
end

---------------------------------------------------
-- Yamaha GEW series PCM
---------------------------------------------------

if SOUNDS["GEW7"]~=null or SOUNDS["MULTIPCM"] then
	files {
		MAME_DIR .. "src/devices/sound/gew.cpp",
		MAME_DIR .. "src/devices/sound/gew.h",
	}
end

if SOUNDS["GEW7"] then
	files {
		MAME_DIR .. "src/devices/sound/gew7.cpp",
		MAME_DIR .. "src/devices/sound/gew7.h",
	}
end

if SOUNDS["MULTIPCM"] then
	files {
		MAME_DIR .. "src/devices/sound/multipcm.cpp",
		MAME_DIR .. "src/devices/sound/multipcm.h",
	}
end

---------------------------------------------------
-- MP3 AUDIO
---------------------------------------------------

if SOUNDS["MP3_AUDIO"] then
	files {
		MAME_DIR .. "src/devices/sound/mp3_audio.cpp",
		MAME_DIR .. "src/devices/sound/mp3_audio.h",
	}
end

---------------------------------------------------
-- MPEG AUDIO
---------------------------------------------------

if SOUNDS["MPEG_AUDIO"] then
	files {
		MAME_DIR .. "src/devices/sound/mpeg_audio.cpp",
		MAME_DIR .. "src/devices/sound/mpeg_audio.h",
	}
end

---------------------------------------------------
-- ZOOM ZSG-2
---------------------------------------------------

if SOUNDS["ZSG2"] then
	files {
		MAME_DIR .. "src/devices/sound/zsg2.cpp",
		MAME_DIR .. "src/devices/sound/zsg2.h",
	}
end

---------------------------------------------------
-- VRC6
---------------------------------------------------

if SOUNDS["VRC6"] then
	files {
		MAME_DIR .. "src/devices/sound/vrc6.cpp",
		MAME_DIR .. "src/devices/sound/vrc6.h",
	}
end

---------------------------------------------------
-- AD1848
---------------------------------------------------

if SOUNDS["AD1848"] then
	files {
		MAME_DIR .. "src/devices/sound/ad1848.cpp",
		MAME_DIR .. "src/devices/sound/ad1848.h",
	}
end

---------------------------------------------------
-- GB_SOUND
---------------------------------------------------

if SOUNDS["GB_SOUND"] then
	files {
		MAME_DIR .. "src/devices/sound/gb.cpp",
		MAME_DIR .. "src/devices/sound/gb.h",
	}
end

---------------------------------------------------
-- PCD3311
---------------------------------------------------

if SOUNDS["PCD3311"] then
	files {
		MAME_DIR .. "src/devices/sound/pcd3311.cpp",
		MAME_DIR .. "src/devices/sound/pcd3311.h",
	}
end

---------------------------------------------------
-- DAC-76 COMDAC
---------------------------------------------------
if SOUNDS["DAC76"] then
	files {
		MAME_DIR .. "src/devices/sound/dac76.cpp",
		MAME_DIR .. "src/devices/sound/dac76.h",
	}
end

---------------------------------------------------
-- MM5837 Noise Generator
---------------------------------------------------

if SOUNDS["MM5837"] then
	files {
		MAME_DIR .. "src/devices/sound/mm5837.cpp",
		MAME_DIR .. "src/devices/sound/mm5837.h",
	}
end

---------------------------------------------------
-- Toshiba TA7630
---------------------------------------------------

if SOUNDS["TA7630"] then
	files {
		MAME_DIR .. "src/devices/sound/ta7630.cpp",
		MAME_DIR .. "src/devices/sound/ta7630.h",
	}
end

---------------------------------------------------
-- Sanyo LC7535
---------------------------------------------------

if SOUNDS["LC7535"] then
	files {
		MAME_DIR .. "src/devices/sound/lc7535.cpp",
		MAME_DIR .. "src/devices/sound/lc7535.h",
	}
end

---------------------------------------------------
-- Sanyo LC78836M
---------------------------------------------------

if SOUNDS["LC78836M"] then
	files {
		MAME_DIR .. "src/devices/sound/lc78836m.cpp",
		MAME_DIR .. "src/devices/sound/lc78836m.h",
	}
end

---------------------------------------------------
-- Sanyo LC82310
---------------------------------------------------

if SOUNDS["LC82310"] then
	files {
		MAME_DIR .. "src/devices/sound/lc82310.cpp",
		MAME_DIR .. "src/devices/sound/lc82310.h",
	}
end

---------------------------------------------------
-- NEC uPD931
---------------------------------------------------

if SOUNDS["UPD931"] then
	files {
		MAME_DIR .. "src/devices/sound/upd931.cpp",
		MAME_DIR .. "src/devices/sound/upd931.h",
	}
end

---------------------------------------------------
-- NEC uPD933
---------------------------------------------------

if SOUNDS["UPD933"] then
	files {
		MAME_DIR .. "src/devices/sound/upd933.cpp",
		MAME_DIR .. "src/devices/sound/upd933.h",
	}
end

---------------------------------------------------
-- NEC uPD934G
---------------------------------------------------

if SOUNDS["UPD934G"] then
	files {
		MAME_DIR .. "src/devices/sound/upd934g.cpp",
		MAME_DIR .. "src/devices/sound/upd934g.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["IOPSPU"] then
	files {
		MAME_DIR .. "src/devices/sound/iopspu.cpp",
		MAME_DIR .. "src/devices/sound/iopspu.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["SWP00"] then
	files {
		MAME_DIR .. "src/devices/sound/swp00.cpp",
		MAME_DIR .. "src/devices/sound/swp00.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["SWP20"] then
	files {
		MAME_DIR .. "src/devices/sound/swp20.cpp",
		MAME_DIR .. "src/devices/sound/swp20.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["SWX00"] then
	files {
		MAME_DIR .. "src/devices/sound/swx00.cpp",
		MAME_DIR .. "src/devices/sound/swx00.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["MEG"] then
	files {
		MAME_DIR .. "src/devices/sound/meg.cpp",
		MAME_DIR .. "src/devices/sound/meg.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["XT446"] then
	files {
		MAME_DIR .. "src/devices/sound/xt446.cpp",
		MAME_DIR .. "src/devices/sound/xt446.h",
	}
end

---------------------------------------------------
-- Roland GP-based sample players
---------------------------------------------------

if SOUNDS["ROLANDGP"] then
	files {
		MAME_DIR .. "src/devices/sound/roland_gp.cpp",
		MAME_DIR .. "src/devices/sound/roland_gp.h",
	}
end

---------------------------------------------------
-- Roland LP-based sample players
---------------------------------------------------

if SOUNDS["ROLANDLP"] then
	files {
		MAME_DIR .. "src/devices/sound/roland_lp.cpp",
		MAME_DIR .. "src/devices/sound/roland_lp.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["VGMVIZ"] then
	files {
		MAME_DIR .. "src/devices/sound/vgm_visualizer.cpp",
		MAME_DIR .. "src/devices/sound/vgm_visualizer.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["S_DSP"] then
	files {
		MAME_DIR .. "src/devices/sound/s_dsp.cpp",
		MAME_DIR .. "src/devices/sound/s_dsp.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["KS0164"] then
	files {
		MAME_DIR .. "src/devices/sound/ks0164.cpp",
		MAME_DIR .. "src/devices/sound/ks0164.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["RP2C33_SOUND"] then
	files {
		MAME_DIR .. "src/devices/sound/rp2c33_snd.cpp",
		MAME_DIR .. "src/devices/sound/rp2c33_snd.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["TT5665"] then
	files {
		MAME_DIR .. "src/devices/sound/tt5665.cpp",
		MAME_DIR .. "src/devices/sound/tt5665.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["UDA1344"] then
	files {
		MAME_DIR .. "src/devices/sound/uda1344.cpp",
		MAME_DIR .. "src/devices/sound/uda1344.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["LYNX"] then
	files {
		MAME_DIR .. "src/devices/sound/lynx.cpp",
		MAME_DIR .. "src/devices/sound/lynx.h",
	}
end

---------------------------------------------------
--
---------------------------------------------------

if SOUNDS["NN71003F"] then
	files {
		MAME_DIR .. "src/devices/sound/nn71003f.cpp",
		MAME_DIR .. "src/devices/sound/nn71003f.h",
	}
end

---------------------------------------------------
-- AP2010
---------------------------------------------------

if SOUNDS["AP2010"] then
	files {
		MAME_DIR .. "src/devices/sound/ap2010pcm.cpp",
		MAME_DIR .. "src/devices/sound/ap2010pcm.h",
	}
end

---------------------------------------------------
-- Texas Instruments CF61909
---------------------------------------------------

if SOUNDS["CF61909"] then
	files {
		MAME_DIR .. "src/devices/sound/cf61909.cpp",
		MAME_DIR .. "src/devices/sound/cf61909.h",
	}
end

---------------------------------------------------
-- NEC uPD65043GF-U01
---------------------------------------------------

if SOUNDS["UPD65043GFU01"] then
	files {
		MAME_DIR .. "src/devices/sound/upd65043gfu01.cpp",
		MAME_DIR .. "src/devices/sound/upd65043gfu01.h",
	}
end

---------------------------------------------------
-- Casio GT155
---------------------------------------------------

if SOUNDS["GT155"] then
	files {
		MAME_DIR .. "src/devices/sound/gt155.cpp",
		MAME_DIR .. "src/devices/sound/gt155.h",
	}
end

---------------------------------------------------
-- Nintendo MMC5 Sound
---------------------------------------------------

if SOUNDS["MMC5"] then
	files {
		MAME_DIR .. "src/devices/sound/mmc5.cpp",
		MAME_DIR .. "src/devices/sound/mmc5.h",
	}
end

---------------------------------------------------
-- ADCs
---------------------------------------------------

if SOUNDS["ADC"] then
	files {
		MAME_DIR .. "src/devices/sound/adc.cpp",
		MAME_DIR .. "src/devices/sound/adc.h",
	}
end

---------------------------------------------------
-- Casio FZ-series PCM
---------------------------------------------------

if SOUNDS["FZ_PCM"] then
	files {
		MAME_DIR .. "src/devices/sound/fz_pcm.cpp",
		MAME_DIR .. "src/devices/sound/fz_pcm.h",
	}
end

---------------------------------------------------
-- Akai L6009
---------------------------------------------------

if SOUNDS["L6009"] then
	files {
		MAME_DIR .. "src/devices/sound/l6009.cpp",
		MAME_DIR .. "src/devices/sound/l6009.h",
	}
end
