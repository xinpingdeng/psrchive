/***************************************************************************
 *
 * // Unload_CovarianceMatrix :: For storing covariance matrix data in COV_MAT PSRFITS Binary Table
 *
 ***************************************************************************/

#include "Pulsar/FITSArchive.h" 
#include "Pulsar/CovarianceMatrix.h" 

#include "psrfitsio.h" 
#include <stdlib.h>  

using namespace std;

void Pulsar::FITSArchive::unload (fitsfile* fptr, const CovarianceMatrix* covar) 
{
  if (verbose > 2)
    cerr << "FITSArchive::unload CovarianceMatrix entered" << endl;
    
  // Move and Clear existing rows in COV_MAT 
  psrfits_move_hdu (fptr, "COV_MAT");

  if (verbose > 2) 
    cerr << "FITSArchive::unload CovarianceMatrix"
      " nbin=" << covar->get_nbin() << " npol=" << covar->get_npol() << endl;
    
  // Update nbin value in NBIN (COV_MAT)
  psrfits_update_key (fptr, "NBIN", covar->get_nbin());
  psrfits_update_key (fptr, "NPOL", covar->get_npol());

  vector<unsigned> dimensions;  
    
  // Write Matrix data into the DATA binary table (COV_MAT)
  psrfits_write_col (fptr, "DATA", 1, covar->get_data(), dimensions);

  if (verbose > 2)       
    cerr << "FITSArchive::unload CovarianceMatrix exiting" << endl;
}
