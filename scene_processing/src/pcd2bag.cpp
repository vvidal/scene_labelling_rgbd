/* 
 * File:   pcd2bag.cpp
 * Author: aa755
 *
 * Created on May 18, 2011, 3:17 PM
 */

#include "ros/ros.h"
#include <rosbag/bag.h>
//#include <pcl_tf/transforms.h>
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>
//#include <image_geometry/pinhole_camera_model.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/PointCloud2.h>
#include "pcl/point_types.h"
#include "float.h"
#include "math.h"
#include <iostream>
#include <fstream>
#include "pcl/io/pcd_io.h"
#include "pcl/point_types.h"
#include "pcl/filters/passthrough.h"
#include "pcl/filters/extract_indices.h"
#include "pcl/features/intensity_spin.h"
#include "pcl/features/normal_3d.h"

#include "sensor_msgs/point_cloud_conversion.h"
#include "color.cpp"
#include "pcl/kdtree/kdtree.h"
#include "pcl/kdtree/tree_types.h"
using namespace std;
typedef pcl::PointXYZRGBCamSL PointT;

/*
 * 
 */
int
main (int argc, char** argv)
{
  ros::Time::init();
    rosbag::Bag bag_;
    sensor_msgs::PointCloud2 cloud_blob;
    sensor_msgs::PointCloud2Ptr cloud_out_blob(new sensor_msgs::PointCloud2());
   // sensor_msgs::PointCloud2Ptr cloud_out_blob;
    pcl::PointCloud<PointT> cloud;
    pcl::PointCloud<PointT> frame_cloud;
    
    if (pcl::io::loadPCDFile(argv[1], cloud_blob) == -1) {
        ROS_ERROR("Couldn't read file test_pcd.pcd");
        return (-1);
    }
    ROS_INFO("Loaded %d data points from test_pcd.pcd with the following fields: %s", (int) (cloud_blob.width * cloud_blob.height), pcl::getFieldsList(cloud_blob).c_str());

   pcl::fromROSMsg (cloud_blob, cloud);
    string outFile =string(argv[1])+".bag";
  bag_.open(outFile, rosbag::bagmode::Write);
  
    frame_cloud.header=cloud.header;
   assert(cloud.size ()==cloud.points.size ());
   frame_cloud.points.clear ();
   int i=0;
   int cur_frame_num=0;
   int frame_num;
   
   do
     {
       frame_num=cloud.points[i].cameraIndex;
       if(frame_num==cur_frame_num)
         frame_cloud.points.push_back (cloud.points[i]);
       else
         {
           pcl::toROSMsg (frame_cloud,*cloud_out_blob);
           bag_.write ( "/rgbdslam/my_clouds" , ros::Time::now (), cloud_out_blob );
           cur_frame_num++;
           frame_cloud.points.clear ();
           //write more dummy frames to simulate visibilityVerged algo's requirement of skipping 4 frames
           pcl::toROSMsg (frame_cloud,*cloud_out_blob);
           
           for(int j=0;j<4;j++)
             bag_.write ( "/rgbdslam/my_clouds" , ros::Time::now (), cloud_out_blob );
           
//                   cur_frame_num++;
           while(cur_frame_num<frame_num)
             { // write emply clouds for missing frame indices
                   pcl::toROSMsg (frame_cloud,*cloud_out_blob);
                   bag_.write ( "/rgbdslam/my_clouds" , ros::Time::now (), cloud_out_blob );  
           
                   cur_frame_num++;
           //write more dummy frames to simulate visibilityVerged algo's requirement of skipping 4 frames
           for(int j=0;j<4;j++)
             bag_.write ( "/rgbdslam/my_clouds" , ros::Time::now (), cloud_out_blob );
             }

                frame_cloud.points.push_back (cloud.points[i]);
               assert( cur_frame_num==frame_num);
         }
       i++;
     }while(i<cloud.size ());
     assert(frame_cloud.size ()==0 || frame_cloud.size ()==307200);
     if(frame_cloud.size ()==307200)
       {
                   pcl::toROSMsg (frame_cloud,*cloud_out_blob);
                   bag_.write ( "/rgbdslam/my_clouds" , ros::Time::now (), cloud_out_blob );  
         
       }
       
     
             
             

  return 0;
}
