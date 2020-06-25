COMPONENT_SRCDIRS := iconv
LIBS += iconv
COMPONENT_ADD_LDFLAGS += -L $(COMPONENT_PATH)/iconv/lib $(addprefix -l,$(LIBS))