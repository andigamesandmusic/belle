CPP=g++
FT=freetype-2.8
FT_DIST=http://download.savannah.gnu.org/releases/freetype/$(FT).tar.gz
BIN_DIR=bin
INCLUDE_DIR=include
TOOL_DIR=tool
EXTERNAL_DIR=external
DEFINITIONS_DIR=definitions
FT_DIR=$(EXTERNAL_DIR)/$(FT)
FONTS_DIR=$(EXTERNAL_DIR)/fonts
BELLE_DEPS=$(INCLUDE_DIR)/belle*.h $(PRIM_DEPS)
PRIM_DEPS=$(INCLUDE_DIR)/prim*.h
FT_TAR_GZ=$(EXTERNAL_DIR)/$(FT).tar.gz
FT_BUILT=$(EXTERNAL_DIR)/freetype
DEMO=$(wildcard $(TOOL_DIR)/demo-*.cpp)
TOOL_DEMO=$(DEMO:$(TOOL_DIR)/demo-%.cpp=$(BIN_DIR)/%)
ifeq ($(STRICT),1)
  WARNING_FLAGS=-Weverything -Wno-double-promotion -Wno-comma -Werror
else
  WARNING_FLAGS=-Wall -Wextra
endif
FT_FLAGS=-I$(FT_BUILT)/include -L$(FT_BUILT)/lib -lfreetype
GOOGLE_FONT=https://github.com/google/fonts/blob/master
CPP_LINE=$(CPP) -pthread -I$(INCLUDE_DIR) $(WARNING_FLAGS)
DOWNLOAD=curl -Ls -o $(2) $(1)

engrave: $(BIN_DIR)/engrave

all: engrave demo

demo: $(TOOL_DEMO)

test: $(BIN_DIR)/engrave $(BIN_DIR)/units
	$(BIN_DIR)/units
	$(BIN_DIR)/engrave --test

format: $(BIN_DIR)/sanitize
	bin/sanitize --indentblanklines=no tool/* include/prim* include/belle*

clean:
	$(info * Cleaning)
	rm -rf `cat .gitignore`

.PHONY: engrave all demo test format clean

$(FT_BUILT):
	$(info * Downloading and building FreeType distribution)
	rm -rf $(FT_DIR) $(FT_TAR_GZ) $(FT_BUILT); mkdir -p $(EXTERNAL_DIR)
	$(call DOWNLOAD,$(FT_DIST),$(FT_TAR_GZ))
	cd $(EXTERNAL_DIR); tar -xf $(FT).tar.gz
	rm -f $(FT_TAR_GZ)
	cd $(FT_DIR); ./configure --prefix=`pwd`/ft --without-bzip2 \
	  --without-harfbuzz --without-png --without-zlib > /dev/null
	cd $(FT_DIR); $(MAKE) -j4 > /dev/null
	cd $(FT_DIR); $(MAKE) install > /dev/null
	mkdir -p $(FT_BUILT)/lib
	cp -R $(FT_DIR)/ft/include/freetype2 $(FT_BUILT)/include
	cp -R $(FT_DIR)/ft/lib/libfreetype.a $(FT_BUILT)/lib
	rm -rf $(FT_DIR)

$(FONTS_DIR)/otf:
	$(info * Downloading Merriweather and Bravura fonts)
	rm -rf $(FONTS_DIR)/otf $(FONTS_DIR)/downloaded
	mkdir -p $(FONTS_DIR)/downloaded
	$(call DOWNLOAD,                                                        \
	  $(GOOGLE_FONT)/ofl/merriweather/Merriweather-Regular.ttf?raw=true,    \
	  $(FONTS_DIR)/downloaded/font-text-regular.otf)
	$(call DOWNLOAD,                                                        \
	  $(GOOGLE_FONT)/ofl/merriweather/Merriweather-Bold.ttf?raw=true,       \
	  $(FONTS_DIR)/downloaded/font-text-bold.otf)
	$(call DOWNLOAD,                                                        \
	  $(GOOGLE_FONT)/ofl/merriweather/Merriweather-Italic.ttf?raw=true,     \
	  $(FONTS_DIR)/downloaded/font-text-italic.otf)
	$(call DOWNLOAD,                                                        \
	  $(GOOGLE_FONT)/ofl/merriweather/Merriweather-BoldItalic.ttf?raw=true, \
	  $(FONTS_DIR)/downloaded/font-text-bold-italic.otf)
	$(call DOWNLOAD,http://www.smufl.org/files/bravura-1.204.zip,           \
	  $(FONTS_DIR)/downloaded/bravura.zip)
	cd $(FONTS_DIR)/downloaded; unzip -o -j bravura.zip otf/Bravura.otf
	cd $(FONTS_DIR)/downloaded; rm bravura.zip
	cd $(FONTS_DIR)/downloaded; mv Bravura.otf font-notation.otf
	mv $(FONTS_DIR)/downloaded $(FONTS_DIR)/otf

$(FONTS_DIR)/json: $(FONTS_DIR)/otf bin/convert-font
	$(info * Making fonts)
	rm -rf $(FONTS_DIR)/converted $(FONTS_DIR)/json
	mkdir -p $(FONTS_DIR)/converted
	bin/convert-font $(FONTS_DIR)/otf/font-text-regular.otf \
	                 $(FONTS_DIR)/converted/font-text-regular.json
	bin/convert-font $(FONTS_DIR)/otf/font-text-bold.otf \
	                 $(FONTS_DIR)/converted/font-text-bold.json
	bin/convert-font $(FONTS_DIR)/otf/font-text-italic.otf \
	                 $(FONTS_DIR)/converted/font-text-italic.json
	bin/convert-font $(FONTS_DIR)/otf/font-text-bold-italic.otf \
	                 $(FONTS_DIR)/converted/font-text-bold-italic.json
	bin/convert-font $(FONTS_DIR)/otf/font-notation.otf \
	                 $(FONTS_DIR)/converted/font-notation.json --smufl
	mv $(FONTS_DIR)/converted $(FONTS_DIR)/json

$(INCLUDE_DIR)/mica.h: $(BIN_DIR)/mica $(DEFINITIONS_DIR)/*.xml
	$(info * Building MICA definitions)
	bin/mica

$(INCLUDE_DIR)/resources.h: $(BIN_DIR)/embed $(FONTS_DIR)/json
	$(info * Creating embeddable resource file)
	bin/embed resources $(FONTS_DIR)/json Resources
	mv Resources.h include/resources.h

$(BIN_DIR)/%: $(TOOL_DIR)/demo-%.cpp $(BELLE_DEPS) $(INCLUDE_DIR)/mica.h \
	          $(INCLUDE_DIR)/resources.h
	$(info * Building $@)
	mkdir -p $(BIN_DIR); rm -f $@
	$(CPP_LINE) -o $@ $<

$(BIN_DIR)/engrave: $(TOOL_DIR)/util-engrave.cpp $(BELLE_DEPS) \
	                $(INCLUDE_DIR)/mica.h $(INCLUDE_DIR)/resources.h
	$(info * Building engrave)
	mkdir -p $(BIN_DIR); rm -f $(BIN_DIR)/engrave
	$(CPP_LINE) -o $(BIN_DIR)/engrave $<

$(BIN_DIR)/embed: $(TOOL_DIR)/util-embed.cpp $(PRIM_DEPS)
	$(info * Building embed)
	mkdir -p $(BIN_DIR); rm -f $(BIN_DIR)/embed
	$(CPP_LINE) -o $(BIN_DIR)/embed $<

$(BIN_DIR)/convert-font: $(TOOL_DIR)/util-convert-font.cpp \
	                     $(FT_BUILT) $(BELLE_DEPS) $(INCLUDE_DIR)/mica.h
	$(info * Building convert-font)
	mkdir -p $(BIN_DIR); rm -f $(BIN_DIR)/convert-font
	$(CPP_LINE) -o $(BIN_DIR)/convert-font $< $(FT_FLAGS)

$(BIN_DIR)/mica: $(TOOL_DIR)/util-mica.cpp $(PRIM_DEPS)
	$(info * Building mica)
	mkdir -p $(BIN_DIR); rm -f $(BIN_DIR)/mica
	$(CPP_LINE) -o $(BIN_DIR)/mica $<
