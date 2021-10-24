// DIP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
// 8uc3 = 8bit, u = unsigned, c3 = 3 channles
// gray_32fc1_img = 32float, 1 channel 

// B G R 

#define ANISO 0
#define GAMMA 0
#define BLUR 0
#define DFT 1

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))


/*

+-------- cols
|
|
|
|
rows
*/

static float inverse_lerp(float value, float min, float max) // 0 - 1
{
	float val = (value - min) / (max - min);
	return val;
}

static double inverse_lerp_d(double value, double min, double max)
{
	double val = (value - min) / (max - min);
	return val;
}


int box_blur[9] =
{
	1, 1, 1,
	1, 1, 1,
	1, 1, 1
};

int gaussian_3x3[9] =
{
	1, 2, 1,
	2, 4, 2,
	1, 2, 1
};

int gaussian_5x5[25] =
{
	1, 4,  6,  4,  1,
	4, 16, 24, 16, 4,
	6, 24, 36, 24, 6,
	4, 16, 24, 16, 4,
	1, 4,  6,  4,  1
};

int from_box_matrix(cv::Mat img, int* matrix, int center_x, int center_y)
{
	int MATRIX_DIMENSION = 3;
	int values[9];
	//int* values = new int[3*3];

	for (int y = 0; y < MATRIX_DIMENSION; y++)
		for (int x = 0; x < MATRIX_DIMENSION; x++)
		{
			int y__ = center_y - 1 + y;
			int x__ = center_x - 1 + x;
			bool outside = (
				(y__) < 0 ||
				(x__) < 0 ||
				(x__) >= img.cols * 3 ||
				(y__) >= img.rows);

			if (!outside)
				values[(y * MATRIX_DIMENSION) + x] = matrix[(y * MATRIX_DIMENSION) + x] * img.at<uchar>(y__, x__);
			else
				values[(y * MATRIX_DIMENSION) + x] = 0;
		}

	int final_value = 0;
	for (int i = 0; i < (MATRIX_DIMENSION * MATRIX_DIMENSION); i++)
		final_value += values[i];

	int divide_by = 0;
	for (int i = 0; i < (MATRIX_DIMENSION * MATRIX_DIMENSION); i++)
		divide_by += matrix[i];
	return final_value / divide_by;

}

// only on gray_scale images
void blur_image(cv::Mat src_img, cv::Mat result, int* matrix, std::string name)
{
	// for each pixel
	for (int y = 0; y < src_img.rows - 1; y++)
		for (int x = 0; x < src_img.cols - 1; x++)
		{
			uchar final_pixel_value = from_box_matrix(src_img, matrix, x, y);
			src_img.at<uchar>(y, x) = final_pixel_value;
		}
cv:imshow(name, src_img);
}

void DFT_(cv::Mat origin)
{
	cv::Mat origin_DFT_;
	cv::Mat DFT_BGR2GRAY;
	cv::Mat DFT_BGR2GRAY_result;

	cv::cvtColor(origin, origin_DFT_, CV_BGR2GRAY);
	origin_DFT_.convertTo(DFT_BGR2GRAY, CV_64FC1, 1.0 / 255.0);
	origin_DFT_.convertTo(DFT_BGR2GRAY_result, CV_64FC1, 1.0 / 255.0);

	// DFT
	int height = DFT_BGR2GRAY.rows;
	int width = DFT_BGR2GRAY.cols;
	double scale = 1.0 / sqrt(height * width);

	for (int l = 0; l < height; l++)
		for (int k = 0; k < width; k++)
		{
			std::cout << "x: " << k << " l: " << l << std::endl;
			//f(m, n) + base(m,n)
			double real = 0.0;
			double comp = 0.0;

			// basis
			for (int n = 0; n < height; n++)
				for (int m = 0; m < width; m++)
				{
					double fmn = DFT_BGR2GRAY.at<double>(n, m);
					double argument = 2 * M_PI * (((m * k) / (double)height) + ((n * l) / (double)width));
					argument *= -1;
					real += cos(argument) * fmn;
					comp += sin(argument) * fmn;
				}

			//real = real * scale;
			//comp = comp * scale;

			double phase = atan(comp / real);
			double spec = sqrt(real * real + comp * comp);
			double power = log(spec);

			DFT_BGR2GRAY_result.at<double>(l, k) = power;

		}


	// normalize the value
	double min__ = 0.0;
	double max__ = 0.0;
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			double value = DFT_BGR2GRAY_result.at<double>(y, x);
			if (value < min__)
				min__ = value;

			if (value > max__)
				max__ = value;
		}
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			double value = DFT_BGR2GRAY_result.at<double>(y, x);
			DFT_BGR2GRAY_result.at<double>(y, x) = inverse_lerp_d(value, min__, max__);
		}

	// change 1st and 3rd quadrant 
	// change 2nd and 4th quadrant
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			// 3rd
			if (x >= width / 2 && y >= height / 2)
			{
				double tmp = DFT_BGR2GRAY_result.at<double>(y, x);
				int x_ = x - width / 2;
				int y_ = y - height / 2;

				std::swap(DFT_BGR2GRAY_result.at<double>(y, x), DFT_BGR2GRAY_result.at<double>(y_, x_));
			}
			// 4th
			if (x >= width / 2 && y < height / 2)
			{
				double tmp = DFT_BGR2GRAY_result.at<double>(y, x);
				int x_ = x - width / 2;
				int y_ = y + height / 2;

				std::swap(DFT_BGR2GRAY_result.at<double>(y, x), DFT_BGR2GRAY_result.at<double>(y_, x_));
			}
		}

	{
		using namespace std;
		using namespace cv;
		Mat resized_up;
		int scale = 4;
		resize(DFT_BGR2GRAY_result, resized_up, Size(64 * scale, 64 * scale), INTER_LINEAR);

		imshow("DFT_origin", DFT_BGR2GRAY);
		imshow("result_scaled", resized_up);
		imshow("result", DFT_BGR2GRAY_result);
	}

}

void ANISO_(cv::Mat origin)
{
	// anisotropic diffusion
	// TODO: anisotropic diffusion: max it can do RN is 100 (idk why.. mb its cuz of memory leak or smth? idk)
	int NUMBER_OF_ITERATIONS = 100;
	cv::Mat origin_aniso_;
	cv::cvtColor(origin, origin_aniso_, CV_BGR2GRAY);
	cv::Mat origin_aniso_next;
	cv::cvtColor(origin, origin_aniso_next, CV_BGR2GRAY);

	cv::Mat origin_aniso_64b1c_origin;
	cv::Mat origin_aniso_64b1c;
	cv::Mat origin_aniso_64b1c_next;
	origin_aniso_.convertTo(origin_aniso_64b1c, CV_64FC1, 1.0 / 255.0);
	origin_aniso_.convertTo(origin_aniso_64b1c_next, CV_64FC1, 1.0 / 255.0);
	origin_aniso_.convertTo(origin_aniso_64b1c_origin, CV_64FC1, 1.0 / 255.0);

	// go for each pixel

	for (int i = 0; i < NUMBER_OF_ITERATIONS; i++)
	{
		std::cout << "iter: " << i << std::endl;
		for (int y = 1; y < origin_aniso_64b1c.rows - 2; y++)
			for (int x = 1; x < origin_aniso_64b1c.cols - 2; x++)
			{
				double I = origin_aniso_64b1c.at<double>(y, x);
				double N = origin_aniso_64b1c.at<double>(y, x + 1);
				double S = origin_aniso_64b1c.at<double>(y, x - 1);
				double W = origin_aniso_64b1c.at<double>(y + 1, x);
				double E = origin_aniso_64b1c.at<double>(y - 1, x);


				// CAN convert uchar to double
				double o = 0.015;
				double l = 0.1;

				//double cI = exp(-1 * (pow(abs(I), 2) / pow(o, 2)));
				double cN = exp(-1 * (pow(abs(N - I), 2) / pow(o, 2)));
				double cS = exp(-1 * (pow(abs(S - I), 2) / pow(o, 2)));
				double cW = exp(-1 * (pow(abs(W - I), 2) / pow(o, 2)));
				double cE = exp(-1 * (pow(abs(E - I), 2) / pow(o, 2)));

				double next_iter =
					(I * (1 - l * (cN + cS + cW + cE))) +
					(l * (cN * N + cS * S + cE * E + cW * W));

				origin_aniso_64b1c_next.at<double>(y, x) = next_iter;
			}
		// reset the prev image and get ready for next iteration
		for (int y = 1; y < origin_aniso_64b1c.rows - 2; y++)
			for (int x = 1; x < origin_aniso_64b1c.cols - 2; x++)
				origin_aniso_64b1c.at<double>(y, x) = origin_aniso_64b1c_next.at<double>(y, x);
	}

	cv::imshow("aniso_origin", origin_aniso_64b1c_origin);
	cv::imshow("next_aniso", origin_aniso_64b1c);

}

int main()
{


#if GAMMA
	cv::Mat src_8uc3_moon = cv::imread("images/moon.jpg", CV_LOAD_IMAGE_COLOR);
	cv::Mat src_8uc3_moon_pow0_5 = cv::imread("images/moon.jpg", CV_LOAD_IMAGE_COLOR);
	cv::Mat src_8uc3_moon_pow2 = cv::imread("images/moon.jpg", CV_LOAD_IMAGE_COLOR);

	for (int y = 0; y < src_8uc3_moon.rows; y++)
		for (int x = 0; x < src_8uc3_moon.cols * 3; x++)
		{
			uchar value = inverse_lerp(src_8uc3_moon.at<uchar>(y, x), 96, 160) * 255; // value = <0, 255>
			src_8uc3_moon.at<uchar>(y, x) = pow(value, 1);
			src_8uc3_moon_pow2.at<uchar>(y, x) = pow(value, 2);
			float val = (float)sqrt(value);
			src_8uc3_moon_pow0_5.at<uchar>(y, x) = val * 50;
		}

	cv::imshow("pow0.5 _brighter_", src_8uc3_moon_pow0_5);
	cv::imshow("pow2", src_8uc3_moon_pow2);
	cv::imshow("pow1", src_8uc3_moon);
#endif

#if DFT
	cv::Mat origin_DFT = cv::imread("images/lena64.png");
	DFT_(origin_DFT);
#endif

#if ANISO
	cv::Mat origin_aniso = cv::imread("images/valve.png", CV_LOAD_IMAGE_COLOR);
	ANISO_(origin_aniso);
#endif

#if BLUR
	// the convolution
	cv::Mat random = cv::imread("images/distorted_panorama.jpg", CV_LOAD_IMAGE_COLOR);
	cv::Mat random_;
	cv::cvtColor(random, random_, CV_BGR2GRAY);
	cv::Mat random_box;
	cv::cvtColor(random, random_box, CV_BGR2GRAY);
	cv::Mat random_gauss_3x3;
	cv::cvtColor(random, random_gauss_3x3, CV_BGR2GRAY);
	cv::Mat random_gauss_5x5;
	cv::cvtColor(random, random_gauss_5x5, CV_BGR2GRAY);


	cv::imshow("random_original", random_);
	blur_image(random_, random_box, box_blur, "box");
	blur_image(random_, random_gauss_3x3, gaussian_3x3, "gauss3x3");
	blur_image(random_, random_gauss_5x5, gaussian_5x5, "gauss5x5");
#endif


#if 0
	for (int y = 1; y < origin_aniso_64b1c_next.rows - 2; y++)
		for (int x = 1; x < origin_aniso_64b1c_next.cols - 2; x++)
		{
			if (origin_aniso_64b1c.at<double>(y, x) != origin_aniso_64b1c_origin.at<double>(y, x))
				origin_aniso_64b1c.at<double>(y, x) = 0;


			//std::cout << origin_aniso_64b1c_next.at<double>(y, x) << " - " << origin_aniso_64b1c_origin.at<double>(y, x) << std::endl;
		}
#endif

	// tohle je nic
#if 0
	// conolution
	for (int y = 0; y < src_8uc3_moon.rows; y++)
		for (int x = 0; x < src_8uc3_moon.cols * 3; x++)
		{
			// MxM = 3x3
			const int MAT_SIZE = 9;
			int values[MAT_SIZE];
			int slot = -1;
			// get values
			for (int y_ = 0; y_ < sqrt(MAT_SIZE); y_++)
				for (int x_ = 0; x_ < sqrt(MAT_SIZE); x_++)
				{
					slot += 1;
					uchar value = 0;
					values[slot] = value;
					if ((x - x_) < 0 || (y - y_) < 0 || (x + x_) > src_8uc3_moon.cols * 3 || (y + y_) > src_8uc3_moon.rows)
					{
					}
					else
					{
						value = src_8uc3_moon.at<uchar>(y - y_, x - x_);
						values[slot] = value;
					}
				}

			// final value = sum of _values_
			int final_pixel_value = 0;
			for (int i = 0; i < MAT_SIZE; i++)
				final_pixel_value += values[i];
			final_pixel_value = final_pixel_value / MAT_SIZE;

			src_8uc3_moon.at<uchar>(y, x) = final_pixel_value;
		}
	cv::imshow("blur", src_8uc3_moon);
#endif

#if 0
	cv::Mat src_8uc3_img = cv::imread("images/lena.png", CV_LOAD_IMAGE_COLOR); // load color image from file system to Mat variable, this will be loaded using 8 bits (uchar)

	if (src_8uc3_img.empty()) {
		printf("Unable to read input file (%s, %d).", __FILE__, __LINE__);
	}

	for (int y = 0; y < src_32fc3_moon.rows; y++)
		for (int x = 0; x < src_32fc3_moon.cols * 3; x++)
		{
			float x_ = src_32fc3_moon.at<float>(y, x);
			src_32fc3_moon.at<float>(y, x) = inverse_lerp(src_32fc3_moon.at<float>(y, x), 96 / 255, 160 / 255);
		}

	//cv::imshow("moon_", src_32fc3_moon);

	//cv::imshow( "LENA", src_8uc3_img );

	cv::Mat gray_8uc1_img; // declare variable to hold grayscale version of img variable, gray levels wil be represented using 8 bits (uchar)
	cv::Mat gray_32fc1_img; // declare variable to hold grayscale version of img variable, gray levels wil be represented using 32 bits (float)

	cv::cvtColor(src_8uc3_img, gray_8uc1_img, CV_BGR2GRAY); // convert input color image to grayscale one, CV_BGR2GRAY specifies direction of conversion
	gray_8uc1_img.convertTo(gray_32fc1_img, CV_32FC1, 1.0 / 255.0); // convert grayscale image from 8 bits to 32 bits, resulting values will be in the interval 0.0 - 1.0

	int x = 10, y = 15; // pixel coordinates

	uchar p1 = gray_8uc1_img.at<uchar>(y, x); // read grayscale value of a pixel, image represented using 8 bits
	float p2 = gray_32fc1_img.at<float>(y, x); // read grayscale value of a pixel, image represented using 32 bits
	cv::Vec3b p3 = src_8uc3_img.at<cv::Vec3b>(y, x); // read color value of a pixel, image represented using 8 bits per color channel

	// print values of pixels
	printf("p1 = %d\n", p1);
	printf("p2 = %f\n", p2);
	printf("p3[ 0 ] = %d, p3[ 1 ] = %d, p3[ 2 ] = %d\n", p3[0], p3[1], p3[2]);

	gray_8uc1_img.at<uchar>(y, x) = 0; // set pixel value to 0 (black)

	// draw a rectangle
	cv::rectangle(gray_8uc1_img, cv::Point(65, 84), cv::Point(75, 94),
		cv::Scalar(50), CV_FILLED);

	// declare variable to hold gradient image with dimensions: width= 256 pixels, height= 50 pixels.
	// Gray levels wil be represented using 8 bits (uchar)
	cv::Mat gradient_8uc1_img(50, 1024, CV_8UC1);

	// For every pixel in image, assign a brightness value according to the x coordinate.
	// This wil create a horizontal gradient.
	for (int y = 0; y < gradient_8uc1_img.rows; y++) {
		for (int x = 0; x < gradient_8uc1_img.cols; x++) {
			uchar a = gradient_8uc1_img.at<uchar>(y, x);
			//float b = gradient_8uc1_img.at<float>(y, x);
			gradient_8uc1_img.at<uchar>(y, x) = x / 4;
		}
	}


	//cv::imshow("te", gradient_8uc1_img);
	// diplay images
	//cv::imshow("Gradient", gradient_8uc1_img);
	//cv::imshow("Lena gray", gray_8uc1_img);
	//cv::imshow("Lena gray 32f", gray_32fc1_img);
#endif
	cv::waitKey(0); // wait until keypressed

	return 0;
}
