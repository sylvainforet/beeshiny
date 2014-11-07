#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>

cv::Mat extractKeypoints(std::string path, int num_images){
    std::vector<cv::KeyPoint> keypoints; 
    cv::Mat descriptors;
    cv::Mat all_features;
    cv::SiftFeatureDetector detector;
    cv::SiftDescriptorExtractor extractor;
    //BRISK extractor;

    for(int i=0;i<num_images;i++){
        cv::Mat input_img;
        std::string img_name = path+std::to_string(i)+".jpg";
        input_img = cv::imread(img_name, CV_LOAD_IMAGE_GRAYSCALE);
        if (!input_img.data || input_img.empty()){
            std::cout << "Error with input image" << img_name << std::endl;
            //return -1;
        }
        //else {
        //  cout << "Frame image of depth " << input_img.depth() << endl;
        //}
        //cout << img_name << endl;
        detector.detect(input_img, keypoints);
        extractor.compute(input_img,keypoints,descriptors);
        //cout << descriptors << endl;
        //cout << descriptors.size() << endl;
        all_features.push_back(descriptors);
    }
    return all_features;
}

int main ( )
{
/*
//.clone().reshape(0,1);

    std::string t1_folder = "/Users/u5305887/Movies/Output/tags/1/";
    std::string t2_folder = "/Users/u5305887/Movies/Output/tags/2/";
    std::string t3_folder = "/Users/u5305887/Movies/Output/tags/3/";
    std::string t4_folder = "/Users/u5305887/Movies/Output/tags/4/";
    std::string t5_folder = "/Users/u5305887/Movies/Output/tags/5/";
    std::string t6_folder = "/Users/u5305887/Movies/Output/tags/6/";
    std::string t7_folder = "/Users/u5305887/Movies/Output/tags/7/";
    std::string t8_folder = "/Users/u5305887/Movies/Output/tags/8/";
    std::string A_folder = "/Users/u5305887/Movies/Output/tags/a_u/";
    std::string clove_folder = "/Users/u5305887/Movies/Output/tags/clove/";
    std::string G_folder = "/Users/u5305887/Movies/Output/tags/g_u/";
    std::string H_folder = "/Users/u5305887/Movies/Output/tags/h_u/";
    std::string N_folder = "/Users/u5305887/Movies/Output/tags/n_u/";
    std::string y_folder = "/Users/u5305887/Movies/Output/tags/y_l/";

    

    cv::Mat t1_m = extractKeypoints(t1_folder,159480);
    cv::Mat t2_m = extractKeypoints(t2_folder,127800);
    cv::Mat t3_m = extractKeypoints(t3_folder,159840);
    cv::Mat t4_m = extractKeypoints(t4_folder,159840);
    cv::Mat t5_m = extractKeypoints(t5_folder,159840);
    cv::Mat t6_m = extractKeypoints(t6_folder,159840);
    cv::Mat t7_m = extractKeypoints(t7_folder,158760);
    cv::Mat t8_m = extractKeypoints(t8_folder,159840);

    cv::Mat A_m = extractKeypoints(A_folder,159840);
    cv::Mat clove_m = extractKeypoints(clove_folder,74880);
    cv::Mat G_m = extractKeypoints(G_folder,159840);
    cv::Mat H_m = extractKeypoints(H_folder,158040);
    cv::Mat N_m = extractKeypoints(N_folder,134280);
    cv::Mat y_m = extractKeypoints(y_folder,62280);


    std::vector <float> labels;
    cv::Mat labelsMat;

    std::cout << t1_m.rows << std::endl;
    std::cout << t2_m.rows << std::endl;
    std::cout << t3_m.rows << std::endl;
    std::cout << t4_m.rows << std::endl;
    std::cout << t5_m.rows << std::endl;
    std::cout << t6_m.rows << std::endl;
    std::cout << t7_m.rows << std::endl;
    std::cout << t8_m.rows << std::endl;
    std::cout << A_m.rows << std::endl;
    std::cout << clove_m.rows << std::endl;
    std::cout << G_m.rows << std::endl;
    std::cout << H_m.rows << std::endl;
    std::cout << N_m.rows << std::endl;
    std::cout << y_m.rows << std::endl;


    for(int i=0;i<t1_m.rows;i++){
        labelsMat.push_back(1.0);
    }
    for(int i=0;i<t2_m.rows;i++){
        labelsMat.push_back(2.0);
    }
    for(int i=0;i<t3_m.rows;i++){
        labelsMat.push_back(3.0);
    }
    for(int i=0;i<t4_m.rows;i++){
        labelsMat.push_back(4.0);
    }
    for(int i=0;i<t5_m.rows;i++){
        labelsMat.push_back(5.0);
    }
    for(int i=0;i<t6_m.rows;i++){
        labelsMat.push_back(6.0);
    }
    for(int i=0;i<t7_m.rows;i++){
        labelsMat.push_back(7.0);
    }
    for(int i=0;i<t8_m.rows;i++){
        labelsMat.push_back(8.0);
    }
    for(int i=0;i<A_m.rows;i++){
        labelsMat.push_back(9.0);
    }
    for(int i=0;i<clove_m.rows;i++){
        labelsMat.push_back(10.0);
    }
    for(int i=0;i<G_m.rows;i++){
        labelsMat.push_back(11.0);
    }
    for(int i=0;i<H_m.rows;i++){
        labelsMat.push_back(12.0);
    }
    for(int i=0;i<N_m.rows;i++){
        labelsMat.push_back(13.0);
    }
    for(int i=0;i<y_m.rows;i++){
        labelsMat.push_back(14.0);
    }

    t1_m.push_back(t2_m);
    t1_m.push_back(t3_m);
    t1_m.push_back(t4_m);
    t1_m.push_back(t5_m);
    t1_m.push_back(t6_m);
    t1_m.push_back(t7_m);
    t1_m.push_back(t8_m);
    t1_m.push_back(A_m);
    t1_m.push_back(clove_m);
    t1_m.push_back(G_m);
    t1_m.push_back(H_m);
    t1_m.push_back(N_m);
    t1_m.push_back(y_m);
    

    //cv::Mat labelsMat(labels.size(),1,CV_32FC1);
    //std::cout << M.rows << std::endl;
    //std::cout << labelsMat.rows << std::endl;

    t1_m.convertTo(t1_m, CV_32F);
    labelsMat.convertTo(labelsMat, CV_32F);

    CvSVMParams params;
    params.svm_type    = CvSVM::C_SVC;
    params.kernel_type = CvSVM::LINEAR;
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

    CvSVM SVM;
    SVM.train(t1_m, labelsMat, cv::Mat(), cv::Mat(), params);

    SVM.save("tags.yml");
*/
    CvSVM SVM;
    SVM.load("tags.yml");

    cv::Mat testing = cv::imread("/Users/u5305887/Movies/Output/train_test.jpg", CV_LOAD_IMAGE_GRAYSCALE);

    std::vector<cv::KeyPoint> keypoints; 
    cv::Mat descriptors;
    cv::SiftFeatureDetector detector;
    cv::SiftDescriptorExtractor extractor;
    detector.detect(testing, keypoints);
    extractor.compute(testing,keypoints,descriptors);

    std::cout << SVM.predict(descriptors.row(0)) << std::endl;






    /*

    for(int i=0;i<19000;i++){
        bee_back.push_back(bees.row(i));
    }
    for(int i=0;i<19000;i++){
        bee_back.push_back(background.row(i));
    }


    float labels[38000];
    for(int i=0;i<38000;i++){
        if (i < 19000){
            labels[i] = 1.0;
        }
        else{
            labels[i] = -1.0;
        }
    }

    Mat labelsMat(38000,1,CV_32FC1,labels);

    CvSVMParams params;
    params.svm_type    = CvSVM::C_SVC;
    params.kernel_type = CvSVM::LINEAR;
    params.term_crit   = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

    CvSVM SVM;
    SVM.train(bee_back, labelsMat, Mat(), Mat(), params);

    SVM.save("bee_svm.yml");
*/



    return 0;
}