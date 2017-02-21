/**
 * \file LinearClusterRemoval.h
 *
 * \ingroup Clusterer
 * 
 * \brief Class def header for a class LinearClusterRemoval
 *
 * @author david caratelli
 */

/** \addtogroup Clusterer

    @{*/

#ifndef LARLITE_LINEARCLUSTERREMOVAL_H
#define LARLITE_LINEARCLUSTERREMOVAL_H

#include "Analysis/ana_base.h"
#include "DataFormat/hit.h"

#include "TwoDimTools/Linearity.h"

#include <map>

namespace larlite {
  /**
     \class LinearClusterRemoval
     User custom analysis class made by SHELL_USER_NAME
   */
  class LinearClusterRemoval : public ana_base{
  
  public:

    /// Default constructor
    LinearClusterRemoval();

    /// Default destructor
    virtual ~LinearClusterRemoval(){}

    /** IMPLEMENT in LinearClusterRemoval.cc!
        Initialization method to be called before the analysis event loop.
    */ 
    virtual bool initialize();

    /** IMPLEMENT in LinearClusterRemoval.cc! 
        Analyze a data event-by-event  
    */
    virtual bool analyze(storage_manager* storage);

    /** IMPLEMENT in LinearClusterRemoval.cc! 
        Finalize method to be called after all events processed.
    */
    virtual bool finalize();

    /// set maximum linearity allowed for this
    void setMaxLinearity(double l) { _max_lin_v.push_back( l ); }
    /// Verbosity setter
    void setVerbose(bool on) { _verbose = on; }
    void setDebug  (bool on) { _debug   = on; }
    
    /// set minimum number of hits
    void setMinNHits(int n) { _min_n_hits_v.push_back( n ); }

    /// Set Producers
    void setClusterProducer(std::string s) { _clusterProducer = s; }
    void setOutClusterProducer(std::string s) { _out_clusterProducer = s; }

  protected:

    /// maximum linearity for hits
    std::vector<double> _max_lin_v;

    // min number of hits for cluster to be considered for removal
    std::vector<int>    _min_n_hits_v;
    
    /// verbosity flag
    bool _verbose;
    /// debug flag
    bool _debug;

    /// conversion factors for hits
    double _wire2cm, _time2cm;

    /// producers
    std::string _clusterProducer;
    std::string _out_clusterProducer;

    /// covariance, standard deviation, mean
    double cov (const std::vector<double>& data1,
		const std::vector<double>& data2) const;
    double stdev(const std::vector<double>& data) const;
    double mean (const std::vector<double>& data) const;
    double linearity(const std::vector<double>& data1,
		     const std::vector<double>& data2) const;

    TTree* _tree;
    int _nhits;
    double _lin;
    double _local_lin_truncated;
    double _local_lin_avg;
    
  };
}
#endif

//**************************************************************************
// 
// For Analysis framework documentation, read Manual.pdf here:
//
// http://microboone-docdb.fnal.gov:8080/cgi-bin/ShowDocument?docid=3183
//
//**************************************************************************

/** @} */ // end of doxygen group 
