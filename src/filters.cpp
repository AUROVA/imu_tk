/* 
 * imu_tk - Inertial Measurement Unit Toolkit
 * 
 *  Copyright (c) 2014, Alberto Pretto <pretto@diag.uniroma1.it>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 * 
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include "imu_tk/filters.h"

using namespace Eigen;

template <typename _T> 
  void imu_tk::staticIntervalsDetector ( const std::vector< imu_tk::TriadData_<_T> >& samples, 
                                         std::vector< imu_tk::DataInterval >& intervals )
{
  intervals.clear();

  imu_tk::DataInterval current_interval(-1, -1);
  int previous_id = -1;

  for( int i = 0; i < samples.size(); i++ )
  {

    assert ( samples[i].interval_id() >= 0 && "Error in staticIntervalsDetector, negative interval id!");

    if ( samples[i].interval_id() != previous_id )
    {
      if( current_interval.start_idx != -1)
      {
        intervals.push_back(current_interval);
      }
      current_interval.start_idx = i;
      previous_id = samples[i].interval_id();
    }
    
    if ( samples[i].interval_id() == previous_id )
    {
       current_interval.end_idx = i;
    }
  }
  intervals.push_back(current_interval);
}


template void imu_tk::staticIntervalsDetector<double> ( const std::vector< TriadData_<double> > &samples,
                                                        std::vector< DataInterval > &intervals);
template void imu_tk::staticIntervalsDetector<float> ( const std::vector< TriadData_<float> > &samples,
                                                        std::vector< DataInterval > &intervals);
