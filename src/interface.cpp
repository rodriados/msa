/** @file interface.cpp
 * @brief Parallel Multiple Sequence Alignment command line interface file.
 * @author Rodrigo Siqueira <rodriados@gmail.com>
 * @copyright 2018 Rodrigo Siqueira
 */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>

#include "msa.h"
#include "interface.hpp"

clidata_t cli_data;
clicommand_t cli_command[] = {
    {CLI_HELP, "-h", "--help",    "Displays this help menu."}
,   {CLI_VERS, "-v", "--version", "Displays the version information."}
,   {CLI_VERB, "-b", "--verbose", "Activates the verbose mode."}
,   {CLI_FILE, "-f", "--file",    "File to be loaded into application.", "fn"}
,   {CLI_UNKN}
};

using namespace std;

/** @fn void file(int, char **, int *)
 * @brief Gets the name of file to be loaded and processed.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments.
 * @param i The current command line argument index.
 */
void file(int argc, char **argv, int *i)
{
    if(argc <= *i + 1)
        finish(NOFILE);

    cli_data.fname = argv[++*i];
}

/** @fn void help(char *)
 * @brief Prints out the help menu.
 * @param pname The name used to start the software.
 */
void help(char *pname)
{
    stringstream ss;

    cerr << "Usage: "  << pname << " [options] -f fn" << endl;
    cerr << "Options:" << endl;

    for(int i = 0; cli_command[i].abb; ++i) {
        ss << cli_command[i].full << ' ' << cli_command[i].arg;
        cerr << setw(4) << right << cli_command[i].abb   << ", "
             << setw(15) << left  << ss.str()
             << cli_command[i].desc << endl;
        ss.str(string());
    }

    finish(NOERROR);
}

/** @fn void version()
 * @brief Prints out the software version number.
 */
void version()
{
    cerr << setw(4) << left << MSA << VERSION << endl;
    finish(NOERROR);
}

/** @fn void vmode()
 * @brief Activates the verbose mode to print execution information.
 */
void vmode()
{
    verbose = 1;
    __debugh("verbose mode on\n");
}

/** @fn void unknown(char *, char *)
 * @brief Informs the user an unknown argument was used.
 * @param pname The name used to start the software.
 * @param comm The unknown command detected.
 */
void unknown(char *pname, char *comm)
{
    cerr << "Unknown option: " << comm << endl;
    cerr << "Try `" << pname << " -h' for more information." << endl;
    finish(NOERROR);
}

/** @fn clicommand_t *search(const char *)
 * @brief Searches for a command in the command list.
 * @param comm Command to be searched for.
 * @return The selected command.
 */
clicommand_t *search(const char *comm)
{
    int i;

    for(i = 0; cli_command[i].abb || cli_command[i].full; ++i)
        if(!strcmp(cli_command[i].abb, comm) || !strcmp(cli_command[i].full, comm))
            break;

    return &cli_command[i];
}

/** @fn void parsecli(int, char **)
 * @brief Parses the command line arguments and fills up the command line struct.
 * @param argc Number of arguments sent by command line.
 * @param argv The arguments sent by command line.
 */
void parsecli(int argc, char **argv)
{
    for(int i = 1; i < argc; ++i)
        switch(search(argv[i])->id) {
            case CLI_HELP: help(argv[0]);               break;
            case CLI_VERS: version();                   break;
            case CLI_VERB: vmode();                     break;
            case CLI_FILE: file(argc, argv, &i);        break;
            case CLI_UNKN: unknown(argv[0], argv[i]);   break;
        }

    if(!cli_data.fname)
        finish(NOFILE);
}
