tokenizer.cc: $(srcdir)/tokenizer.l
	flex -+ -o$@ $<

CLEANFILES = tokenizer.cc

noinst_LTLIBRARIES = libclassparser.la
INCLUDES = $(all_includes)
libclassparser_la_METASOURCES = AUTO

libclassparser_la_SOURCES = tokenizer.cc PersistantClassStore.cc ParsedStruct.cc ParsedSignalSlot.cc ParsedScopeContainer.cc ParsedParent.cc ParsedMethod.cc ParsedItem.cc ParsedContainer.cc ParsedClassContainer.cc ParsedClass.cc ParsedAttribute.cc ParsedArgument.cc ClassTreeNode.cc ClassStore.cc ClassParser.cc 
