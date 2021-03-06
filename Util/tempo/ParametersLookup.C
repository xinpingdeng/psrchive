/***************************************************************************
 *
 *   Copyright (C) 2009 by Willem van Straten
 *   Licensed under the Academic Free License version 2.1
 *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Pulsar/ParametersLookup.h"
#include "TemporaryDirectory.h"
#include "DirectoryLock.h"
#include "load_factory.h"
#include "tempo++.h"
#include "dirutil.h"

#include <stdlib.h>
#include <unistd.h>

using namespace std;

Pulsar::Parameters::Lookup::Lookup () 
{
  ext.push_back (".eph");
  ext.push_back (".par");

  try
  {
    string tempo_pardir = Tempo::get_configuration("PARDIR");
    if (tempo_pardir.length())
      path.push_back (tempo_pardir);
  }
  catch (Error& error)
  {
    if (verbose)
      cerr << "Pulsar::Parameters::Lookup error " << error.get_message() << endl;
  }
}

void Pulsar::Parameters::Lookup::add_extension (const string& e)
{
  if (!e.length())
    return;

  string edot = e;

  if (e[0] != '.')
    edot = "." + e;

  ext.push_back (edot);
}    

void Pulsar::Parameters::Lookup::add_path (const string& p)
{
  path.push_back (p);
}

static TemporaryDirectory directory ("psrcat");
static DirectoryLock dir_lock;

Pulsar::Parameters* 
Pulsar::Parameters::Lookup::operator() (const string& name) const try
{
  if (!name.length())
    throw Error (InvalidParam, "Pulsar::Parameters::Lookup::operator()",
		 "empty name");

  string psr_name = name;

  if (name[0] == 'J' || name[0] == 'B')
    psr_name.erase (0,1);

  for (unsigned ipath=0; ipath<path.size(); ipath++)
  {
    for (unsigned iext=0; iext < ext.size(); iext++)
    {
      string filename = path[ipath] + psr_name + ext[iext];

      if (verbose)
	cerr << "Pulsar::Parameters::Lookup '" << filename << "'" << endl;

      if (file_exists(filename.c_str()))
      {
	if (verbose)
	  cerr << "Pulsar::Parameters::Lookup using " << filename << endl;

	return factory<Pulsar::Parameters> (filename);
      }
    }
  }
  
  /* Create name.eph in local directory */ 

#ifdef HAVE_PSRCAT

  string catalogue = "psrcat";
  string command = catalogue;

  if (getenv("PSRCAT_RUNDIR") != 0)
    command += " -all ";

  command += " -e " + psr_name + " > " + psr_name + ".eph";

#else

  string command = "psrinfo -e " + psr_name;
  string catalogue = "psrinfo";

#endif

  if (verbose)
    cerr << "Pulsar::Parameters::Lookup:: Creating ephemeris by " << catalogue 
	 << " -e " << psr_name <<endl;

  dir_lock.set_directory( directory.get_directory() );
  DirectoryLock::Push raii (dir_lock);

  // start with a clean working directory
  dir_lock.clean ();

  int retval = system(command.c_str());

  if (retval != 0)
    throw Error (FailedSys, "Pulsar::Parameters::Lookup::operator()"
	"system (" + command + ")");

  vector<string> filenames;
  dirglob (&filenames, "*.eph");

  if (filenames.size() != 1)
    throw Error (InvalidState, "Pulsar::Parameters::Lookup::operator()",
		 "%s created %d files", catalogue.c_str(), filenames.size());

  string filename = filenames[0];

  if (file_exists(filename.c_str()))
  {
    if (verbose)
      cerr << "Pulsar::Parameters::Lookup using '" + filename + "'" << endl;

    Pulsar::Parameters * params = factory<Pulsar::Parameters> (filename);
    return params;
  }

  throw Error (InvalidParam, "Pulsar::Parameters::Lookup::operator()",
               "Cannot find "+ filename +" after call to "+ catalogue); 
}
catch (Error& error)
{
  throw error += "Pulsar::Parameters::Lookup::operator()";
}
