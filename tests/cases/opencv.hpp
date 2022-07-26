#ifdef OPENCV
TEST_CASE( "From OPENCV", "[from_opencv]" )
{

    cv::Mat M( 2, 2, CV_8UC3, cv::Scalar(0, 0, 255) );
    std::cout << "OPENCV matrix:\n" << M << std::endl;
    feng::matrix<std::int32_t> mat;
    mat.from_opencv( M );
    std::cout << "Converted to feng::matrix:\n" << mat << std::endl;

    cv::Mat MM = mat.to_opencv( 3 );
    std::cout << "Convert back to OPENCV matrix:\n" << MM << std::endl;

}
#endif
