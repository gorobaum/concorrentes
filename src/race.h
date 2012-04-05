
#ifndef EP1_RACE_H_
#define EP1_RACE_H_

/* Loads the race information from an input file. Once loaded, a race may be run
 * with RACErun only once. Then you have to use RACEcleanup and load again from
 * an input file.
 * Returns 0 upon success and -1 otherwise. */
int
RACEload (const char *inputfile);

/* Displays the currently loaded race's information. */
void
RACEdisplay_info ();

/* Runs the currently loaded race, showing all the results at the end of the
 * simulation.
 * Returns -1 if an error ocurred and 0 otherwise. */
int
RACErun ();

void
RACEreport ();

/* Cleans up all the dinamic memory used by the last race, so that a new one may
 * be loaded through RACEload. */
void
RACEcleanup ();

#endif /* EP1_RACE_H_ */

