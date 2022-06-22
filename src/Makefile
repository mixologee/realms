# Generated automatically from Makefile.in by configure.
# CircleMUD Makefile.in - Makefile template used by 'configure'
#

# C compiler to use
CC = gcc
#CC = g++

# Path to cxref utility
CXREF = cxref

# Any special flags you want to pass to the compiler
MYFLAGS = -Wall

#flags for profiling (see hacker.doc for more information)
PROFILE = 


##############################################################################
# Do Not Modify Anything Below This Line (unless you know what you're doing) #
##############################################################################

BINDIR = ../bin

CFLAGS = -m64 -g $(MYFLAGS) $(PROFILE)

LIBS =  -lcrypt -lgd

CLIBS = -L/usr/lib64/mysql/libmysqlclient_r -I/usr/include/mysql

OBJFILES = act.comm.o act.informative.o act.item.o act.movement.o \
	act.offensive.o act.other.o act.social.o act.wizard.o aedit.o \
	ban.o boards.o bsd-snprintf.o cedit.o class.o comm.o config.o \
	constants.o db.o dg_comm.o dg_db_scripts.o dg_event.o color.o \
	dg_handler.o dg_misc.o dg_mobcmd.o dg_objcmd.o dg_olc.o dg_scripts.o \
	dg_triggers.o dg_variables.o dg_wldcmd.o fight.o genmob.o \
	genobj.o genolc.o genshp.o genwld.o genzon.o graph.o handler.o \
	house.o improved-edit.o interpreter.o limits.o magic.o mail.o medit.o \
	mobact.o modify.o oasis.o oasis_copy.o oasis_delete.o oasis_list.o \
	objsave.o oedit.o players.o random.o redit.o sedit.o shop.o \
	spec_assign.o spec_procs.o spell_parser.o spells.o tedit.o utils.o \
        weather.o zedit.o zmalloc.o clan.o range.o arena.o miner.o \
        godcmds.o commands.o profession.o crystalgate.o survey.o \
        warn.o weaponprocs.o economy.o qedit.o genqst.o quest.o \
        mob_skills.o asciimap.o mob_intelligence.o mob_party.o messanger.o \
        room_afs.o mailedit.o buildings.o goods.o wilderness.o wild.info.o \
        wild.life.o wild.map.o wild.ships.o stables.o vehicles.o room_trigger.o \
	clanedit.o helpsys.o maputils.o logger.o formation.o \
        custprompt.o 
         
CXREF_FILES = act.comm.c act.informative.c act.item.c act.movement.c \
	act.offensive.c act.other.c act.social.c act.wizard.c aedit.c \
	ban.c boards.c bsd-snprintf.c cedit.c class.c comm.c config.c \
	constants.c db.c dg_comm.c dg_db_scripts.c dg_event.c color.c \
	dg_handler.c dg_misc.c dg_mobcmd.c dg_objcmd.c dg_olc.c dg_scripts.c \
	dg_triggers.c dg_variables.c dg_wldcmd.c fight.c genmob.c \
	genobj.c genolc.c genshp.c genwld.c genzon.c graph.c handler.c \
	house.c improved-edit.c interpreter.c limits.c magic.c mail.c medit.c \
	mobact.c modify.c oasis.c oasis_copy.c oasis_delete.c oasis_list.c \
	objsave.c oedit.c players.c random.c redit.c sedit.c shop.c \
	spec_assign.c spec_procs.c spell_parser.c spells.c tedit.c utils.c \
	weather.c zedit.c zmalloc.c clan.c range.c arena.c miner.c \
        godcmds.c commands.c profession.c crystalgate.c survey.c \
        warn.c weaponprocs.c economy.c qedit.c genqst.c quest.c\
	mob_skills.c asciimap.c mob_intelligence.c mob_party.c messanger.c \
        room_afs.c mailedit.c buildings.c goods.c wilderness.c wild.info.c \
        wild.life.c wild.map.c wild.ships.c stables.c vehicles.c room_trigger.c \
	clanedit.c helpsys.c maputils.c logger.c formation.c \
        custpromt.c 

default: all

all: .accepted
	$(MAKE) $(BINDIR)/circle
	$(MAKE) utils

.accepted:
	@./licheck less

utils: .accepted
	(cd util; $(MAKE) all)
circle:
	$(MAKE) $(BINDIR)/circle

$(BINDIR)/circle : $(OBJFILES)
	$(CC) -o $(BINDIR)/circle $(PROFILE) $(OBJFILES) $(LIBS) $(CLIBS)

clean:
	rm -f *.o
ref:
#
# Create the cross reference files
# Note, this is not meant to be used unless you've installed cxref...
#
	@for file in $(CXREF_FILES) ; do \
	  echo Cross referencing $$file ; \
	  $(CXREF) -D__CXREF__ -xref -Odoc -Ncircle $$file ; \
	done
#
# Create the source files using cxref
#
	@for file in $(CXREF_FILES) ; do \
	   echo Documenting $$file ; \
	   ( cd . ; $(CXREF) -D__CXREF__ -warn-xref -xref -Odoc -Ncircle -html $$file ) ; \
	   rm -f $(DOCS) ; \
	done
#
# Create the index using cxref
#
	@echo Indexing
	@( cd . ; $(CXREF) -D__CXREF__ -index-all -Odoc -Ncircle -html )
	@rm -f $(DOCS)
#
# Make html files for the .h files
#
	@echo Creating .h.html files...
	@for file in *.h ; do \
	  echo $$file ; \
	  cat htmlh-head $$file htmlh-tail > doc/$$file.html ; \
	done
# Copy over to the html directory
	#cp doc/*.html $(HOME)/www/cxref
	#chmod 644 $(HOME)/www/cxref/*.html

# Dependencies for the object files (automagically generated with
# gcc -MM)

depend:
	$(CC) -MM *.c > depend

-include depend
