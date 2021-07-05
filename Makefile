
SOURCES += main.cpp
SOURCES += LayerBitmapFontDraw.cpp
SOURCES += CharacterData.cpp CharacterSet.cpp ComplexRect.cpp cp932_uni.cpp FontSystem.cpp FreeType.cpp MiscUtility.cpp PrerenderedFont.cpp TVPSysFont.cpp uni_cp932.cpp
SOURCES += LayerBitmapFontManagement.cpp FreeTypeFontRasterizer.cpp NativeFreeTypeFace.cpp
SOURCES += RectItf.cpp tvpgl.cpp
SOURCES += external/freetype/src/autofit/autofit.c external/freetype/src/bdf/bdf.c external/freetype/src/cff/cff.c external/freetype/src/base/ftbase.c external/freetype/src/base/ftbitmap.c external/freetype/src/cache/ftcache.c external/freetype/builds/windows/ftdebug.c external/freetype/src/base/ftfstype.c external/freetype/src/base/ftgasp.c external/freetype/src/base/ftglyph.c external/freetype/src/gzip/ftgzip.c external/freetype/src/base/ftinit.c external/freetype/src/lzw/ftlzw.c external/freetype/src/base/ftstroke.c external/freetype/src/base/ftsystem.c external/freetype/src/smooth/smooth.c external/freetype/src/base/ftbbox.c external/freetype/src/base/ftfntfmt.c external/freetype/src/base/ftmm.c external/freetype/src/base/ftpfr.c external/freetype/src/base/ftsynth.c external/freetype/src/base/fttype1.c external/freetype/src/base/ftwinfnt.c external/freetype/src/base/ftlcdfil.c external/freetype/src/base/ftgxval.c external/freetype/src/base/ftotval.c external/freetype/src/base/ftpatent.c external/freetype/src/pcf/pcf.c external/freetype/src/pfr/pfr.c external/freetype/src/psaux/psaux.c external/freetype/src/pshinter/pshinter.c external/freetype/src/psnames/psmodule.c external/freetype/src/raster/raster.c external/freetype/src/sfnt/sfnt.c external/freetype/src/truetype/truetype.c external/freetype/src/type1/type1.c external/freetype/src/cid/type1cid.c external/freetype/src/type42/type42.c external/freetype/src/winfonts/winfnt.c
INCFLAGS += -Iexternal/freetype/include
CFLAGS += -DFT2_BUILD_LIBRARY
LDLIBS += -lgdi32
PROJECT_BASENAME = krfreetype

RC_LEGALCOPYRIGHT ?= Copyright (C) 2021-2021 Julian Uy; See details of license at license.txt, or the source code location.

include external/ncbind/Rules.lib.make
