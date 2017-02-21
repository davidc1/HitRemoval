#ifndef LARLITE_CLUSTERFILTER_CXX
#define LARLITE_CLUSTERFILTER_CXX

#include "ClusterFilter.h"

#include "LArUtil/GeometryHelper.h"
#include "LArUtil/Geometry.h"

#include "DataFormat/cluster.h"
#include "DataFormat/vertex.h"
#include "DataFormat/hit.h"

namespace larlite {

  ClusterFilter::ClusterFilter()
  {

    _name = "ClusterFilter";

    _fout = 0;

    _debug = false;
    
    _vtx_w_cm = {0,0,0};
    _vtx_t_cm = {0,0,0};

    _max_n_hits = 200;
    _Amax = 100*100;
    _d_max = 200;

    _clusProducer = "";
    _vtxProducer  = "";
    _out_clusterProducer = "";
    
  }

  bool ClusterFilter::initialize() {

    _wire2cm  = larutil::GeometryHelper::GetME()->WireToCm();
    _time2cm  = larutil::GeometryHelper::GetME()->TimeToCm();

    std::cout << "********************************" << std::endl;
    std::cout << "Wire -> cm conversion : " << _wire2cm << std::endl;
    std::cout << "Time -> cm conversion : " << _time2cm << std::endl;
    std::cout << "********************************" << std::endl;

    return true;
  }
  
  bool ClusterFilter::analyze(storage_manager* storage) {

    auto evt_clus = storage->get_data<event_cluster>(_clusProducer);
    auto evt_vtx  = storage->get_data<event_vertex> (_vtxProducer );
    auto out_clus = storage->get_data<event_cluster>( _out_clusterProducer );
    auto out_ass_cluster_hit_v = storage->get_data<event_ass>(out_clus->name());
    std::vector<std::vector<unsigned int> > out_cluster_hit_ass_v;
    
    larlite::event_hit* evt_hit = nullptr;
    auto const& ass_cluster_hit_v = storage->find_one_ass(evt_clus->id(), evt_hit, evt_clus->name());

    //set event ID through storage manager
    storage->set_id(storage->run_id(),storage->subrun_id(),storage->event_id());

    // get vertex position on each plane
    if ( (_vtxProducer != "") and (evt_vtx->size() == 1) ){
      auto const& vtx = evt_vtx->at(0);
      auto geoH = larutil::GeometryHelper::GetME();
      std::vector<double> xyz = {vtx.X(), vtx.Y(), vtx.Z()};
      for (size_t pl = 0; pl < 3; pl++){
	auto const& pt = geoH->Point_3Dto2D(xyz,pl);
	_vtx_w_cm[pl] = pt.w;
	_vtx_t_cm[pl] = pt.t + 800 * _time2cm;
	if (_debug) std::cout << "Vtx @ plane " << pl << " -> [" << _vtx_w_cm[pl] << ", " << _vtx_t_cm[pl] << std::endl;
      }
    }

    // loop through all clusters and filter based on various criteria
    for (size_t clus_idx; clus_idx < evt_clus->size(); clus_idx++){

      // grab hit indices associated to this cluster
      auto const& hit_idx_v = ass_cluster_hit_v[clus_idx];

      // apply number of hit cut
      if (hit_idx_v.size() > _max_n_hits){
	for (auto const& hit_idx : hit_idx_v)
	  evt_hit->at(hit_idx).set_goodness(-1.0);
	continue;
      }

      int pl = evt_hit->at( hit_idx_v[0] ).WireID().Plane;
      if (_debug) std::cout << "Plane : " << pl << "\t clus size : " << hit_idx_v.size() << std::endl;

      // grab x & y coordinates for cluster
      std::vector<double> w_v, t_v;
      getClusterPoints(hit_idx_v, evt_hit, w_v, t_v);

      // grab cluster bounds (w_min, w_max, t_min, t_max)
      std::pair<double,double> w_bounds;
      std::pair<double,double> t_bounds;
      double d_max, d_min;
      getClusterBounds(w_v, t_v, pl, w_bounds, t_bounds, d_max, d_min);

      if (_debug)
	std::cout << "\t cluster bounds LL [" << w_bounds.first << ", " << t_bounds.first << "]" << std::endl
		  << "\t                LR [" << w_bounds.second << ", " << t_bounds.first << "]" << std::endl
		  << "\t                UR [" << w_bounds.second << ", " << t_bounds.second << "]" << std::endl
		  << "\t                UL [" << w_bounds.first << ", " << t_bounds.second << "]" << std::endl
		  << "\t Max dist from vtx : " << d_max << std::endl
		  << "\t Min dist from vtx : " << d_min << std::endl;
      
      // remove clusters that span only 1 or 2 wires
      // and are ~ a few cm in time (units in cuts are in cm)
      if ( ( (w_bounds.second - w_bounds.first) < 0.5 ) and
	   ( (t_bounds.second - t_bounds.first) > 1.5 ) ){
	if (_debug) std::cout << "remove -> spans too few wires/times" << std::endl;
	for (auto const& hit_idx : hit_idx_v)
	  evt_hit->at(hit_idx).set_goodness(-1.0);
	continue;
      }
      
      // calculate area
      double A = ( w_bounds.second - w_bounds.first ) * ( t_bounds.second - t_bounds.first );
      
      if (_debug) std::cout << "\t cluster area : " << A << std::endl;
      
      // apply area cut
      if (A > _Amax){
	if (_debug) std::cout << "remove -> area too large" << std::endl;
	for (auto const& hit_idx : hit_idx_v)
	  evt_hit->at(hit_idx).set_goodness(-1.0);
	continue;
      }

      // check max distance from vtx
      if ( (d_max > _d_max) && (hit_idx_v.size() > 100) ){
	if (_debug) std::cout << "remove -> too far from vtx" << std::endl;
	for (auto const& hit_idx : hit_idx_v)
	  evt_hit->at(hit_idx).set_goodness(-1.0);
	continue;
      }

      bool drop = false;
      
      twodimtools::Linearity lin(w_v,t_v);
      
      
      if ( (hit_idx_v.size() > 20) and (d_min > 30) and (lin._local_lin_truncated_avg < 0.5) ){
	// calcualte linearity
	
	double slope     = lin._slope;
	double intercept = lin._intercept;
	
	// impact parameter to vertex:
	double x0 = _vtx_w_cm[pl];
	double y0 = _vtx_t_cm[pl];
	double IP = fabs( - slope * x0 + y0 - intercept ) / sqrt( slope * slope + 1 );
	
	if (IP > 20)
	  drop = true;
	
      }
      
      if (drop == true){
	if (_debug) std::cout << "remove -> linearity" << std::endl;
	for (auto const& hit_idx : hit_idx_v)
	  evt_hit->at(hit_idx).set_goodness(-1.0);
	continue;
      }

      // made it this far. save the cluster!
      out_clus->emplace_back( evt_clus->at( clus_idx ) );
      out_cluster_hit_ass_v.push_back( hit_idx_v );

    }// for all clusters in the event

    out_ass_cluster_hit_v->set_association(out_clus->id(),product_id(data::kHit,evt_hit->name()),out_cluster_hit_ass_v);    
      

    return true;
  }

  bool ClusterFilter::finalize() {

    return true;
  }

  void ClusterFilter::getClusterPoints(const std::vector<unsigned int>& hit_idx_v,
				       larlite::event_hit* evt_hit,
				       std::vector<double>& w_v,
				       std::vector<double>& t_v) const
  {

    w_v.clear();
    t_v.clear();

    for (auto const& hit_idx : hit_idx_v){

      w_v.push_back( evt_hit->at(hit_idx).WireID().Wire * _wire2cm );
      t_v.push_back( evt_hit->at(hit_idx).PeakTime()    * _time2cm );
      
    }

    return;
  }

  void ClusterFilter::getClusterBounds(const std::vector<double>& w_v,
				       const std::vector<double>& t_v,
				       const int& pl,
				       std::pair<double,double>& w_bounds,
				       std::pair<double,double>& t_bounds,
				       double& d_max, double& d_min) const
  {

    if (w_v.size() != t_v.size()){
      std::cout << "W and T vectors of different size..." << std::endl;
      return;
    }

    double w_min =  10000;
    double w_max = -10000;
    double t_min =  10000;
    double t_max = -10000;

    d_max = 0;
    d_min = 10000;

    for (size_t n=0; n < w_v.size(); n++){

      double w = w_v[n];
      double t = t_v[n];

      double dist = sqrt ( ( (_vtx_w_cm[pl] - w) * (_vtx_w_cm[pl] - w) ) +
			   ( (_vtx_t_cm[pl] - t) * (_vtx_t_cm[pl] - t) ) );

      if (dist > d_max) d_max = dist;
      if (dist < d_min) d_min = dist;

      if (w > w_max) w_max = w;
      if (w < w_min) w_min = w;
      if (t > t_max) t_max = t;
      if (t < t_min) t_min = t;
      
    }// for all hits

    w_bounds.first  = w_min;
    w_bounds.second = w_max;
    t_bounds.first  = t_min;
    t_bounds.second = t_max;
    
    return;
  }
				      

}
#endif
