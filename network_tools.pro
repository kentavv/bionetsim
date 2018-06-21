TEMPLATE           = subdirs
SUBDIRS            = libfitness \
                     libgui \
                     libnetwork \
                     network_editor


# Not updated yet
#SUBDIRS           += fitness_landscape \
#                     ode_gen \
#                     param_test
#                     ga_breed \
#                     parameter_scan_tools \
#                     report \
#                     steady_state \
#                     extract_rxn_names \
#                     time_series
#                     libplugin \                     

linux-g++:SUBDIRS += fh_evolve \
                     ga_ode_opt

#linux-g++:SUBDIRS += de_ode_opt \
#                     distributed_stochastic \
#                     distributed_stochastic/client \
#                     distributed_stochastic/engine \
#                     ga_ode_opt \
#                     random_simplex \
#                     simplex_test \
#\
#                     graphs \
#                     distributed_stochastic/viewer \
#                     fh_evolve
