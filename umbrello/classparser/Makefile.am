tokenizer.cc: $(srcdir)/tokenizer.l
	@LEX@ -+ -o$@ $<

CLEANFILES = tokenizer.cc

####### kdevelop will overwrite this part!!! (begin)##########
noinst_LIBRARIES = libclassparser.a

INCLUDES = $(all_includes)

libclassparser_a_METASOURCES = AUTO

libclassparser_a_SOURCES = tokenizer.cc PersistantClassStore.cc ParsedStruct.cc ParsedSignalSlot.cc ParsedScopeContainer.cc ParsedParent.cc ParsedMethod.cc ParsedItem.cc ParsedContainer.cc ParsedClassContainer.cc ParsedClass.cc ParsedAttribute.cc ParsedArgument.cc ClassTreeNode.cc ClassStore.cc ClassParser.cc 


EXTRA_DIST = ClassParser.h ClassStore.h ClassTreeNode.h ParsedArgument.h ParsedAttribute.h ParsedClassContainer.h ParsedClass.h ParsedContainer.h ParsedItem.h ParsedMethod.h ParsedParent.h ParsedScopeContainer.h ParsedSignalSlot.h ParsedStruct.h PersistantClassStore.h ProgrammingByContract.h tokenizer.h ClassParser.cc ClassStore.cc ClassTreeNode.cc ParsedArgument.cc ParsedAttribute.cc ParsedClass.cc ParsedClassContainer.cc ParsedContainer.cc ParsedItem.cc ParsedMethod.cc ParsedParent.cc ParsedScopeContainer.cc ParsedSignalSlot.cc ParsedStruct.cc PersistantClassStore.cc tokenizer.cc 

####### kdevelop will overwrite this part!!! (end)############

