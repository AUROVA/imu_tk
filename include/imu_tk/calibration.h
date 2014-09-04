#pragma once

#include <vector>
#include <iostream>
#include <fstream>

#include "imu_tk/base.h"

namespace imu_tk
{
/*
 * Misalignment matrix:
 * 
 * general case:
 * 
 *     [    1     -mis_yz   mis_zy  ]
 * T = [  mis_xz     1     -mis_zx  ]
 *     [ -mis_xy   mis_yx     1     ]
 * 
 * "body" frame spacial case:
 * 
 *     [  1     -mis_yz   mis_zy  ]
 * T = [  0        1     -mis_zx  ]
 *     [  0        0        1     ]
 * 
 * Scale matrix:
 * 
 *     [  s_x      0        0  ]
 * K = [   0      s_y       0  ]
 *     [   0       0       s_z ]
 * 
 * Bias vector:
 * 
 *     [ b_x ]
 * B = [ b_y ]
 *     [ b_z ]
 * 
 * Given a raw sensor reading X (e.g., the acceleration ), the calibrated "unbiased" reading X' is obtained
 * 
 * X' = T*K*(X - B)
 * 
 * with B the bias (variable) + offset (constant, possbibly 0), or, equivalently:
 * 
 * X' = T*K*X - B'
 * 
 * with B' = T*K*B'
 * 
 * Without knowing the value of the bias (and with offset == 0), the calibrated reading X'' is simply:
 * 
 * X'' = T*K*X
 */
template < typename _T = double > class CalibratedTriad
{
public:
  CalibratedTriad( const _T &mis_yz = _T(0), const _T &mis_zy = _T(0), const _T &mis_zx = _T(0), 
                   const _T &mis_xz = _T(0), const _T &mis_xy = _T(0), const _T &mis_yx = _T(0), 
                   const _T &s_x = _T(1),    const _T &s_y = _T(1),    const _T &s_z = _T(1), 
                   const _T &b_x = _T(0),    const _T &b_y = _T(0),    const _T &b_z  = _T(0) );
 
  ~CalibratedTriad(){};
  
  inline _T misYZ() const { return mis_yz_; };
  inline _T misZY() const { return mis_zy_; };
  inline _T misZX() const { return mis_zx_; };
  inline _T misXZ() const { return mis_xz_; };
  inline _T misXY() const { return mis_xy_; };
  inline _T misYX() const { return mis_yx_; };

  inline _T scaleX() const { return s_x_; };
  inline _T scaleY() const { return s_y_; };
  inline _T scaleZ() const { return s_z_; };
      
  inline _T biasX() const { return b_x_; };
  inline _T biasY() const { return b_y_; };
  inline _T biasZ() const { return b_z_; };
  
  inline const Eigen::Matrix< _T, 3 , 3>& getMisalignmentMatrix() const { return mis_mat_; };
  inline const Eigen::Matrix< _T, 3 , 3>& getScaleMatrix() const { return scale_mat_; };
  inline const Eigen::Matrix< _T, 3 , 1>& getBiasVector() const { return bias_vec_; };
    
  inline void setScale( const Eigen::Matrix< _T, 3 , 1> &s_vec ) { s_x_ = s_vec(0); s_y_ = s_vec(1); s_z_ = s_vec(2); };
  inline void setBias( const Eigen::Matrix< _T, 3 , 1> &b_vec ) { b_x_ = b_vec(0); b_y_ = b_vec(1); b_z_ = b_vec(2); };
  
  bool load( std::string filename );
  bool save( std::string filename ) const;

  inline Eigen::Matrix< _T, 3 , 1> normalize( const Eigen::Matrix< _T, 3 , 1> &raw_data )
  {
    return ms_mat_*raw_data;
  };
  
  inline TriadData<_T> normalize( const TriadData<_T> &raw_data )
  {
    return TriadData<_T>( raw_data.timestamp(), normalize( raw_data.data()) );
  };
  
  inline Eigen::Matrix< _T, 3 , 1> unbiasNormalize( const Eigen::Matrix< _T, 3 , 1> &raw_data )
  {
    return ms_mat_*(raw_data - bias_vec_); 
  };
  
  inline TriadData<_T> unbiasNormalize( const TriadData<_T> &raw_data )
  {
    return TriadData<_T>( raw_data.timestamp(), unbiasNormalize( raw_data.data()) );
  };
  
  inline Eigen::Matrix< _T, 3 , 1> unbias( const Eigen::Matrix< _T, 3 , 1> &raw_data )
  {
    return raw_data - bias_vec_; 
  };
  
  inline TriadData<_T> unbias( const TriadData<_T> &raw_data )
  {
    return TriadData<_T>( raw_data.timestamp(), unbias( raw_data.data()) );
  };
  
private:
  // Misalignment parameters
  _T mis_yz_, mis_zy_, mis_zx_, mis_xz_, mis_xy_, mis_yx_;
  // Scale parameters
  _T s_x_, s_y_, s_z_;
  // Biases
  _T b_x_, b_y_, b_z_;
  // Misalignment matrix
  Eigen::Matrix< _T, 3 , 3> mis_mat_;
  // Scale matrix
  Eigen::Matrix< _T, 3 , 3> scale_mat_;
  // Bias vector
  Eigen::Matrix< _T, 3 , 1> bias_vec_;
  // Misalignment * scale matrix
  Eigen::Matrix< _T, 3 , 3> ms_mat_;
};

template <typename _T> std::ostream& operator<<(std::ostream& os, 
                                                const CalibratedTriad<_T>& calib_triad)
{
  os<<"Misalignment Matrix"<<std::endl;
  os<<calib_triad.getMisalignmentMatrix()<<std::endl;
  os<<"Scale Matrix"<<std::endl;
  os<<calib_triad.getScaleMatrix()<<std::endl;
  os<<"Bias Vector"<<std::endl;
  os<<calib_triad.getBiasVector()<<std::endl;
  return os;
}

template <typename _T = double > class MultiPosCalibration
{
public:
  
  MultiPosCalibration();
  ~MultiPosCalibration(){};
  
  _T gravityMagnitede() const { return g_mag_; };
  int numInitSamples() const { return n_init_samples_; };
  int intarvalsNumSamples() const { return interval_n_samples_; };
  const CalibratedTriad<_T>& initAccCalibration(){ return init_acc_calib_; };
  const CalibratedTriad<_T>& initGyroCalibration(){ return init_gyro_calib_; };
  bool accUseMeans() const { return acc_use_means_; };
  _T gyroDataPeriod() const{ return gyro_dt_; };
  bool optimizeGyroBias() const { return optimize_gyro_bias_; };
  bool verboseOutput() const { return verbose_output_; };
  
  void setGravityMagnitude( _T g ){ g_mag_ = g; };
  void setNumInitSamples( int num ) { n_init_samples_ = num; };
  int setIntarvalsNumSamples( int num ) { interval_n_samples_ = num; };
  void setInitAccCalibration( CalibratedTriad<_T> &init_calib ){ init_acc_calib_ = init_calib; };
  void setInitGyroCalibration( CalibratedTriad<_T> &init_calib ){ init_gyro_calib_ = init_calib; };
  void enableAccUseMeans ( bool enabled ){ acc_use_means_ = enabled; };
  void setGyroDataPeriod( _T dt ){ gyro_dt_ = dt; };
  bool enableGyroBiasOptimization( bool enabled  ) { optimize_gyro_bias_ = enabled; };
  void enableVerboseOutput( bool enabled ){ verbose_output_ = enabled; };
  
  bool calibrateAcc( const std::vector< TriadData<_T> > &acc_samples );
  bool calibrateAccGyro( const std::vector< TriadData<_T> > &acc_samples, 
                         const std::vector< TriadData<_T> > &gyro_samples );

  const CalibratedTriad<_T>& getAccCalib() const  { return acc_calib_; };
  const CalibratedTriad<_T>& getGyroCalib() const  { return gyro_calib_; };
  const std::vector< TriadData<_T> >& getCalibAccSamples() const { return calib_acc_samples_; };
  const std::vector< TriadData<_T> >& getCalibGyroSamples() const { return calib_gyro_samples_; };
  
private:
  
  _T g_mag_;
  const int min_num_intervals_;
  int n_init_samples_;
  int interval_n_samples_;
  bool acc_use_means_;
  _T gyro_dt_;
  bool optimize_gyro_bias_;
  std::vector< DataInterval<_T> > min_cost_static_intervals_;
  CalibratedTriad<_T> init_acc_calib_, init_gyro_calib_;
  CalibratedTriad<_T> acc_calib_, gyro_calib_;
  std::vector< TriadData<_T> > calib_acc_samples_, calib_gyro_samples_;
  
  bool verbose_output_;
};

}

/* Implementations */

template <typename _T> 
  imu_tk::CalibratedTriad<_T>::CalibratedTriad( const _T &mis_yz, const _T &mis_zy, const _T &mis_zx, 
                                                const _T &mis_xz, const _T &mis_xy, const _T &mis_yx, 
                                                const _T &s_x, const _T &s_y, const _T &s_z, 
                                                const _T &b_x, const _T &b_y, const _T &b_z ) :
  mis_yz_(mis_yz), mis_zy_(mis_zy), mis_zx_(mis_zx), 
  mis_xz_(mis_xz), mis_xy_(mis_xy), mis_yx_(mis_yx),
  s_x_(s_x), s_y_(s_y), s_z_(s_z),
  b_x_(b_x), b_y_(b_y), b_z_(b_z)
{
  mis_mat_ <<  _T(1)    , -mis_yz_ ,  mis_zy_ ,
                mis_xz_ ,  _T(1)   , -mis_zx_ ,  
               -mis_xy_ ,  mis_yx_ ,  _T(1)   ;
              
  scale_mat_ <<   s_x_ ,   _T(0)  ,  _T(0) ,
                 _T(0) ,    s_y_  ,  _T(0) ,  
                 _T(0) ,   _T(0)  ,   s_z_ ;
                    
  ms_mat_ = mis_mat_*scale_mat_;
                    
  bias_vec_ <<  b_x_ ,
                b_y_ ,
                b_z_ ; 
}

template <typename _T> 
  bool imu_tk::CalibratedTriad<_T>::load( std::string filename )
{
  std::ifstream file( filename.data() );
  if (file.is_open())
  {
    _T mat[9] = {0};
    
    for( int i=0; i<9; i++)
      file >> mat[i];

    mis_mat_ = Eigen::Map< const Eigen::Matrix< _T, 3, 3, Eigen::RowMajor> >(mat);
      
    for( int i=0; i<9; i++)
      file >> mat[i];
    
    scale_mat_ = Eigen::Map< const Eigen::Matrix< _T, 3, 3, Eigen::RowMajor> >(mat);
        
    for( int i=0; i<3; i++)
      file >> mat[i];
    
    bias_vec_ = Eigen::Map< const Eigen::Matrix< _T, 3, 1> >(mat);    
    
    return true;
  }
  return false;  
}

template <typename _T> 
  bool imu_tk::CalibratedTriad<_T>::save( std::string filename ) const
{
  std::ofstream file( filename.data() );
  if (file.is_open())
  {
    file<<mis_mat_<<std::endl<<std::endl
        <<scale_mat_<<std::endl<<std::endl
        <<bias_vec_<<std::endl<<std::endl;
    
    return true;
  }
  return false;  
}