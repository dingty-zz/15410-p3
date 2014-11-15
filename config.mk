###########################################################################
#
#    #####          #######         #######         ######            ###
#   #     #            #            #     #         #     #           ###
#   #                  #            #     #         #     #           ###
#    #####             #            #     #         ######             #
#         #            #            #     #         #
#   #     #            #            #     #         #                 ###
#    #####             #            #######         #                 ###
#
#
# Please read the directions in README and in this config.mk carefully.
# Do -N-O-T- just dump things randomly in here until your kernel builds.
# If you do that, you run an excellent chance of turning in something
# which can't be graded.  If you think the build infrastructure is
# somehow restricting you from doing something you need to do, contact
# the course staff--don't just hit it with a hammer and move on.
#
# [Once you've read this message, please edit it out of your config.mk]
# [Once you've read this message, please edit it out of your config.mk]
# [Once you've read this message, please edit it out of your config.mk]
###########################################################################

###########################################################################
# This is the include file for the make file.
# You should have to edit only this file to get things to build.
###########################################################################

###########################################################################
# Tab stops
###########################################################################
# If you use tabstops set to something other than the international
# standard of eight characters, this is your opportunity to inform
# our print scripts.
TABSTOP = 8

###########################################################################
# The method for acquiring project updates.
###########################################################################
# This should be "afs" for any Andrew machine, "web" for non-andrew machines
# and "offline" for machines with no network access.
#
# "offline" is strongly not recommended as you may miss important project
# updates.
#
UPDATE_METHOD = afs

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# blanked.  Your kernel MUST BOOT AND RUN if 410TESTS and STUDENTTESTS
# are blank.  It would be wise for you to test that this works.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory.
#
410TESTS = slaughter register_test fork_wait  cho2 \
exec_basic ck1 exec_basic_helper peon merchant fork_test1 \
fork_bomb cat cho coolness deschedule_hang getpid_test1 halt_test knife \
loader_test2 print_basic readline_basic remove_pages_test2 actual_wait \
sleep_test1 new_pages \
swexn_basic_test swexn_cookie_monster swexn_dispatch swexn_regs \
swexn_stands_for_swextensible swexn_uninstall_test \
minclone_mem \
wait_getpid work mem_permissions make_crash \
make_crash_helper chow yield_desc_mkrun loader_test1 \
fork_wait_bomb fork_exit_bomb

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory.
#
STUDENTTESTS = a

###########################################################################
# Data files provided by course staff to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the 410user/files
# directory.
#
410FILES =

###########################################################################
# Data files you have created which you wish to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the user/files
# directory.
#
STUDENTFILES =

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = malloc.o panic.o

# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P3" we give you can't build libthrgrp.a.  Once you install your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
#410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = set_status.o vanish.o print.o fork.o new_pages.o readline.o gettid.o yield.o sleep.o exec.o wait.o task_vanish.o misbehave.o readfile.o set_term_color.o set_cursor_pos.o deschedule.o make_runnable.o misbehave.o get_ticks.o getchar.o remove_pages.o swexn.o halt.o get_cursor_pos.o


###########################################################################
# Object files for your automatic stack handling
###########################################################################
AUTOSTACK_OBJS = autostack.o

###########################################################################
# Parts of your kernel
###########################################################################
#
# Kernel object files you want included from 410kern/
#
410KERNEL_OBJS = load_helper.o
#
# Kernel object files you provide in from kern/
#
KERNEL_OBJS = \
kernel.o loader.o malloc_wrappers.o handler_install.o \
datastructure/linked_list.o \
exception/exception_handlers.o exception/exception_handler_wrappers.o exception/exception_handler_real.o\
hardware/hardware_handler_wrappers.o hardware/keyboard.o hardware/timer.o \
hardware/console.o \
locks/atomic_xchange.o locks/mutex.o \
memory/vm_routines.o memory/memory_management.o memory/sys_memory_management.o \
process/process.o process/scheduler.o process/sys_exec.o process/sys_fork.o \
process/sys_life_cycle.o process/do_switch.o process/enter_user_mode.o \
process/life_cycle.o \
syscall/consoleIO.o syscall/sys_consoleIO.o syscall/misc.o syscall/sys_misc.o \
thread/thread_basic.o thread/sys_thread_management.o thread/thread_management.o \

###########################################################################
# WARNING: Do not put **test** programs into the REQPROGS variables.  Your
#          kernel will probably not build in the test harness and you will
#          lose points.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# The shell is a really good thing to keep here.  Don't delete idle
# or init unless you are writing your own, and don't do that unless
# you have a really good reason to do so.
#

410REQPROGS = idle init shell 

###########################################################################
# Mandatory programs whose source is provided by you
###########################################################################
# A list of the programs in user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# Leave this blank unless you are writing custom init/idle/shell programs
# (not generally recommended).  If you use STUDENTREQPROGS so you can
# temporarily run a special debugging version of init/idle/shell, you
# need to be very sure you blank STUDENTREQPROGS before turning your
# kernel in, or else your tweaked version will run and the test harness
# won't.
#
STUDENTREQPROGS =
