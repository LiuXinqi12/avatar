#include <vector>
#include <iostream>
#include <Eigen/Dense>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <pcl/visualization/pcl_visualizer.h>

#include "Avatar.h"
#include "AvatarPCL.h"

// open a gui for interacting with avatar
void __avatarGUI()
{
    using namespace ark;
    // build file names and paths
    AvatarModel model;
    Avatar ava(model);

    const size_t NKEYS = model.numShapeKeys();

    cv::namedWindow("Body Shape");
    cv::namedWindow("Body Pose");
    std::vector<int> pcw(NKEYS, 1000), p_pcw(NKEYS, 0);

    // define some axes
    const Eigen::Vector3d AXISX(1, 0, 0), AXISY(0, 1, 0), AXISZ(0, 0, 1);

    // Body pose control definitions (currently this control system only supports rotation along one axis per body part)
    const std::vector<std::string> CTRL_NAMES       = {"L HIP",      "R HIP",      "L KNEE",      "R KNEE",      "L ANKLE",      "R ANKLE",      "L ELBLW",        "R ELBOW",        "L WRIST",      "R WRIST",      "HEAD",      "SPINE2",     "ROOT"};
    const std::vector<int> CTRL_JNT               = {SmplJoint::L_HIP, SmplJoint::R_HIP, SmplJoint::L_KNEE, SmplJoint::R_KNEE, SmplJoint::L_ANKLE, SmplJoint::R_ANKLE, SmplJoint::L_ELBOW, SmplJoint::R_ELBOW, SmplJoint::L_WRIST, SmplJoint::R_WRIST, SmplJoint::HEAD, SmplJoint::SPINE2, SmplJoint::ROOT_PELVIS};
    const std::vector<Eigen::Vector3d> CTRL_AXIS    = {AXISX,        AXISX,        AXISX,         AXISX,         AXISX,          AXISX,          AXISY,          AXISY,          AXISY,          AXISY,          AXISX,       AXISX,         AXISY};
    const int N_CTRL = (int)CTRL_NAMES.size();

    std::vector<int> ctrlw(N_CTRL, 1000), p_ctrlw(N_CTRL, 0);

    // Body shapekeys are defined in SMPL model files.
    int pifx = 0, pify = 0, picx = 0, picy = 0, pframeID = -1;
    cv::resizeWindow("Body Shape", cv::Size(400, 700));
    cv::resizeWindow("Body Pose", cv::Size(400, 700));
    cv::resizeWindow("Body Scale", cv::Size(400, 700));
    for (int i = 0; i < N_CTRL; ++i) {
        cv::createTrackbar(CTRL_NAMES[i], "Body Pose", &ctrlw[i], 2000);
    }
    for (int i = 0; i < (int)pcw.size(); ++i) {
        cv::createTrackbar("PC" + std::to_string(i), "Body Shape", &pcw[i], 2000);
    }

    auto viewer = pcl::visualization::PCLVisualizer::Ptr(new pcl::visualization::PCLVisualizer("3D Viewport"));

    viewer->initCameraParameters();
    int vp1 = 0;
    viewer->setWindowName("3D View");
    viewer->setBackgroundColor(0, 0, 0);
    // viewer->setCameraClipDistances(0.0, 1000.0);

    volatile bool interrupt = false;
    viewer->registerKeyboardCallback([&interrupt](const pcl::visualization::KeyboardEvent & evt) {
        unsigned char k = evt.getKeyCode();
        if (k == 'Q' || k == 'q' || k == 27) {
            interrupt = true;
        }
    });

    while (!interrupt) {
        bool controlsChanged = false;
        for (int i = 0; i < N_CTRL; ++i) {
            if (ctrlw[i] != p_ctrlw[i]) {
                controlsChanged = true;
                break;
            }
        }
        for (int i = 0; i < (int)pcw.size(); ++i) {
            if (pcw[i] != p_pcw[i]) {
                controlsChanged = true;
                break;
            }
        }
        if (controlsChanged) {
            //viewer->removeAllPointClouds(vp1);
            //viewer->removeAllShapes(vp1);
            ava.update();

            //viewer->removePointCloud("vp1_cloudHM");
            //viewer->addPointCloud<pcl::PointXYZ>(avatar_pcl::getCloud(ava), "vp1_cloudHM", vp1);
            viewer->removePolygonMesh("meshHM");

            auto mesh = ark::avatar_pcl::getMesh(ava);
            viewer->addPolygonMesh(*mesh, "meshHM", vp1);
            //ava.visualize(viewer, "vp1_", vp1);

            for (int i = 0; i < N_CTRL; ++i) {
                double angle = (ctrlw[i] - 1000) / 1000.0 * M_PI;
                if (angle == 0) ava.r[CTRL_JNT[i]].setIdentity();
                else ava.r[CTRL_JNT[i]] = Eigen::AngleAxisd(angle, CTRL_AXIS[i]).toRotationMatrix();
            }

            for (int i = 0; i < (int)pcw.size(); ++i) {
                ava.w[i] = (float)(pcw[i] - 1000) / 500.0;
            }

            ava.p = Eigen::Vector3d(0, 0, 0);
            ava.update();

            for (int k = 0; k < (int) pcw.size(); ++k) {
                p_pcw[k] = pcw[k] = (int) (ava.w[k] * 500.0 + 1000);
                cv::setTrackbarPos("PC" + std::to_string(k), "Body Shape", pcw[k]);
            }

            double prior = ava.model.posePrior.residual(ava.smplParams()).squaredNorm();
            // show pose prior value
            if (!viewer->updateText("-log likelihood: " + std::to_string(prior), 10, 20, 15, 1.0, 1.0, 1.0, "poseprior_disp")) {
                viewer->addText("-log likelihood: " + std::to_string(prior), 10, 20, 15, 1.0, 1.0, 1.0, "poseprior_disp");
            }

            // viewer->removePointCloud("vp1_cloudHM");
            // viewer->addPointCloud<pcl::PointXYZ>(ava.getCloud(), "vp1_cloudHM");
            // ava.visualize(viewer, "vp1_", vp1);
        }
        for (int i = 0; i < N_CTRL; ++i) p_ctrlw[i] = ctrlw[i];
        for (int i = 0; i < (int)pcw.size(); ++i) p_pcw[i] = pcw[i];

        int k = cv::waitKey(1);
        viewer->spinOnce();
        if (k == 'q' || k == 27) break;
    }
}

int main(int argc, char** argv) {
    __avatarGUI();
    return 0;
}