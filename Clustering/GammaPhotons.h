/**
 * \file GammaPhotons.h
 *
 * \ingroup Clusterer
 * 
 * \brief Class def header for a class GammaPhotons
 *
 * @author david caratelli
 */

/** \addtogroup Clusterer

    @{*/

#ifndef LARLITE_GAMMAPHOTONS_H
#define LARLITE_GAMMAPHOTONS_H

#include "Analysis/ana_base.h"
#include "DataFormat/hit.h"
#include <map>

namespace larlite {
  /**
     \class GammaPhotons
     User custom analysis class made by SHELL_USER_NAME
   */
  class GammaPhotons : public ana_base{
  
  public:

    /// Default constructor
    GammaPhotons();

    /// Default destructor
    virtual ~GammaPhotons(){}

    /** IMPLEMENT in GammaPhotons.cc!
        Initialization method to be called before the analysis event loop.
    */ 
    virtual bool initialize();

    /** IMPLEMENT in GammaPhotons.cc! 
        Analyze a data event-by-event  
    */
    virtual bool analyze(storage_manager* storage);

    /** IMPLEMENT in GammaPhotons.cc! 
        Finalize method to be called after all events processed.
    */
    virtual bool finalize();

    /// Set the size of each cell for hit-map
    void setCellSize(double d) { _cellSize = d; }
    /// Set the radius around which to search for hits
    /// if two hits are within this distance of each other
    /// then they go into the same cluster
    void setRadius(double d) { _radius = d; }
    /// Set which plane to select hits from
    void setPlane(int pl) { _plane = pl; }
    /// Verbosity setter
    void setVerbose(bool on) { _verbose = on; }
    
    /// Set Producers
    void setHitProducer(std::string s) { _hitProducer = s; }
    void setVtxProducer(std::string s) { _vtxProducer = s; }
    void setOutClusProducer(std::string s) { _out_clusterProducer = s; }
    
    /// set ROI to use
    void setROIRadius(double r) { _roi_radius = r; }
    /// use vertex?
    void setUseVertex(bool on) { _useVtx = on; }
    /// cut hits based on their fit RMS [ unit is time-ticks ]
    void setMaxHitRMS(double r) { _max_rms = r; }
    /// set min tick
    void setMinTick(int t) { _tick_min = t; }
    void setMaxTick(int t) { _tick_max = t; }
    /// set minimum truncated linearity
    void setMinLin(double d) { _min_lin = d; }
    /// set maximum number of hits for a cluster
    void setMaxClusSize(int n) { _max_clus_size = n; }

  protected:


    std::vector< std::vector<size_t> > getShowerHits(const std::vector<std::vector<unsigned int> >& ass_shr_pfp_v,
                                                     const std::vector<std::vector<unsigned int> >& ass_pfp_cls_v,
                                                     const std::vector<std::vector<unsigned int> >& ass_cls_hit_v,
                                                     larlite::event_hit* ev_hit);


    /// size of each cell [cm]
    double _cellSize;

    /// radius to count charge around [cm]
    double _radius;
    
    /// plane to select hits from
    int _plane;

    /// verbosity flag
    bool _verbose;

    /// conversion factors for hits
    double _wire2cm, _time2cm;

    /// Producers
    std::string _hitProducer;
    std::string _vtxProducer;
    std::string _out_clusterProducer;

    /// Map making function
    void MakeHitMap(const event_hit* hitlist, int plane, const std::vector<std::vector<size_t> >& shr_hit_idx_v );

    /// Functions to decide if two hits should belong to the same cluster or not
    bool HitsCompatible(const hit& h1, const hit& h2);

    /// Function to get neighboring hits (from self + neighoring cells)
    void getNeighboringHits(const std::pair<int,int>& pair, std::vector<size_t>& hitIndices);

    /// check if time overlaps
    bool TimeOverlap(const larlite::hit& h1, const larlite::hit& h2, double& dmin) const;

    /// map connecting coordinate index (i,j) to [h1,h2,h3] (hit index list)
    std::map<std::pair<int,int>, std::vector<size_t> > _hitMap;

    /// maximum i'th and j'th
    int _maxI;
    int _maxJ;

    /// roi radius
    double _roi_radius;

    /// maximum hit RMS allowed
    double _max_rms;

    /// max cluster size
    size_t _max_clus_size;
    /// min truncated local linearity
    double _min_lin;

    /// vertex coordinates
    std::vector<double> _vtx_w_cm;
    std::vector<double> _vtx_t_cm;

    bool _useVtx;

    // min and max ticks allowed
    int _tick_min, _tick_max;

    // vector of all hit indices for already created showers
    std::vector<size_t> _shr_hit_idx_v;

    
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