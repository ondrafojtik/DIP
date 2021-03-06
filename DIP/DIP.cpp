// DIP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
// 8uc3 = 8bit, u = unsigned, c3 = 3 channles
// gray_32fc1_img = 32float, 1 channel 
#define DFT 0 // BAAAAAAAAAAAD

// B G R 

#define ANISO 0
#define GAMMA 0
#define BLUR 0
#define DFT2 0
#define LOW_PASS 0
#define HIGH_PASS 0
#define FILTER_BARS 0
#define EDGE_DETECTION 0

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
	cv::Mat inverse;

	cv::Mat result2fc;
	cv::Mat inverse_result;

	cv::cvtColor(origin, origin_DFT_, CV_BGR2GRAY);
	origin_DFT_.convertTo(DFT_BGR2GRAY, CV_64FC1, 1.0 / 255.0);
	origin_DFT_.convertTo(DFT_BGR2GRAY_result, CV_64FC1, 1.0 / 255.0);
	origin_DFT_.convertTo(inverse, CV_64FC1, 1.0 / 255.0);

	origin_DFT_.convertTo(inverse_result, CV_64FC1, 1.0 / 255.0);
	origin_DFT_.convertTo(result2fc, CV_64FC2, 1.0 / 255.0);

	// DFT
	int height = result2fc.rows;
	int width =  result2fc.cols;
	double scale = 1.0 / sqrt(height * width);

	for (int l = 0; l < height; l++)
		for (int k = 0; k < width; k++)
		{
			// DFT----------------------------------------------------------------------------
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
			//result2fc.at<cv::Vec2d>(l, k) = cv::Vec2d(real, comp);
			
			double phase = atan(comp / real);
			double spec = sqrt(real * real + comp * comp);
			double power = log(spec);
			DFT_BGR2GRAY_result.at<double>(l, k) = power;

			//real = real * scale;
			//comp = comp * scale;

			//DFT_BGR2GRAY_result.at<double>(l, k) = real + comp;

			// iDFT---------------------------------------------------------------------------
			double i_sum = 0.0;

			for (int n = 0; n < height; n++)
				for (int m = 0; m < width; m++)
				{
					double argument = 2 * M_PI * (((m * k) / (double)height) + ((n * l) / (double)width));
					double i_real = cos(argument) * scale;
					double i_comp = sin(argument) * scale;

					i_sum += (real * i_real) - (comp * i_comp);
				}
			inverse.at<double>(l, k) = i_sum;

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
#if 1
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
#endif

	
	{
		using namespace std;
		using namespace cv;
		Mat resized_up;
		int scale = 4;
		resize(DFT_BGR2GRAY_result, resized_up, Size(64 * scale, 64 * scale), INTER_LINEAR);

		imshow("DFT_origin", DFT_BGR2GRAY);
		imshow("result_scaled", resized_up);
		imshow("result", DFT_BGR2GRAY_result);

		Mat i_resized_up;
		resize(inverse, i_resized_up, Size(64 * scale, 64 * scale), INTER_LINEAR);
		imshow("inverse_scaled", i_resized_up);
		imshow("inverse", inverse_result);
	}

}

void DFT_inverse(cv::Mat origin)
{
	cv::Mat DFT_BGR2GRAY;
	cv::Mat DFT_BGR2GRAY_result;

	origin.convertTo(DFT_BGR2GRAY, CV_64FC1, 1.0 / 255.0);
	origin.convertTo(DFT_BGR2GRAY_result, CV_64FC1, 1.0 / 255.0);
	
	for (int i = 0; i < 64; i++)
		for (int y = 0; y < 64; y++)
			DFT_BGR2GRAY_result.at<double>(i, y) = origin.at<double>(i, y);

	int height = origin.rows;
	int width = origin.cols;
	double scale = 1.0 / sqrt(width * height);

	for (int k = 0; k < height; k++)
		for (int l = 0; l < width; l++)
		{
			double sum = 0.0;

			for (int m = 0; m < height; m++)
				for (int n = 0; n < width; n++)
				{
					double fmn = origin.at<double>(m, n);
					double argument = 2 * M_PI * (((m * k) / (double)height) + ((n * l) / (double)width));
					double real = scale * cos(argument);
					double comp = scale * sin(argument);

					sum += real - comp;
				}
			DFT_BGR2GRAY_result.at<double>(k, l) = sum;

		}


	{
		using namespace std;
		using namespace cv;
		Mat resized_up;
		int scale_ = 4;
		resize(DFT_BGR2GRAY_result, resized_up, Size(64 * scale_, 64 * scale_), INTER_LINEAR);
	
		imshow("iDFT_scaled", resized_up);
		imshow("iDFT", DFT_BGR2GRAY_result);
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

struct vec2
{
	float x, y;
};

cv::Mat edge_detection(cv::Mat origin)
{
	cv::Mat result;
	cv::Mat theta_buffer_x;
	cv::Mat theta_buffer_y;
	cv::Mat edge_buffer;
	cv::Mat final_buffer;
	//cv::cvtColor(origin, result, CV_BGR2GRAY);
	

	result = cv::Mat::zeros(origin.size(), origin.type());
	
	theta_buffer_x = cv::Mat::zeros(result.size(), result.type());
	theta_buffer_y = cv::Mat::zeros(result.size(), result.type());
	edge_buffer = cv::Mat::zeros(result.size(), result.type());
	final_buffer = cv::Mat::zeros(result.size(), result.type());

	const int dim = 3;
	int Gx[dim][dim] = { {1, 0, -1}, {2, 0, -2}, {1, 0, -1} };
	int Gy[dim][dim] = { {1, 2, 1}, {0, 0, 0}, {-1, -2, -1} };

	//int Gx[dim][dim] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
	//int Gy[dim][dim] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

	int width = origin.cols*1 - 2;
	int height = origin.rows - 2;
	
	for (int x = 1; x < width - 1; x++)
		for (int y = 1; y < height - 1; y++)
		{
			// convolution Gx * A 
			double grad_x = 0;
			double grad_y = 0;
			for (int i = -1; i < 2; i++)
				for (int j = -1; j < 2; j++)
				{
					int fA = origin.at<uchar>(y+j, x+i);
					grad_x += fA * Gx[i+1][j+1];
					grad_y += fA * Gy[i+1][j+1];
				}

		
			//result.at<uchar>(y, x) = abs(grad_x) + abs(grad_y);
			double value = sqrt(pow(grad_x, 2) + pow(grad_y, 2));
			result.at<uchar>(y, x) = (int)value;

			theta_buffer_x.at<uchar>(y, x) = grad_x;
			theta_buffer_y.at<uchar>(y, x) = grad_y;

			
		}

	cv::imshow("edge_detection", result);


	cv::Mat thinning;
	thinning = cv::Mat::zeros(origin.size(), origin.type());

#if 0
	// edge thinning
	for (int x = 1; x < width - 1; x++)
		for (int y = 1; y < height - 1; y++)
		{
			// look for max value (in 3x3 matrix)
			int max = 0;
			for (int i = -1; i < 2; i++)
				for (int j = -1; j < 2; j++)
				{
					int fA = result.at<uchar>(y + j, x + i);
					if (fA > max)
						max = fA;
				}

			if (result.at<uchar>(y, x) != max)
				result.at<uchar>(y, x) = 0;
			
		}

	// copy
	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
			thinning.at<uchar>(y, x) = result.at<uchar>(y, x);

	const int t1 = 100;
	const int t2 = 100;

#else

	for (int x = 1; x < width - 1; x++)
		for (int y = 1; y < height - 1; y++)
		{
			double grad_x = theta_buffer_x.at<uchar>(y, x);
			double grad_y = theta_buffer_y.at<uchar>(y, x);

			double angle = atan2(grad_y, grad_x) * 180 / M_PI;
			
			if ((angle >= -22.5 && angle <= 22.5) || (angle < -175.5 && angle >= 180))
			{
				if (result.at<uchar>(y, x) >= result.at<uchar>(y + 1, x) &&
					result.at<uchar>(y, x) >= result.at<uchar>(y - 1, x))
				{
					thinning.at<uchar>(y, x) = result.at<uchar>(y, x);
				}
				else
					thinning.at<uchar>(y, x) = 0;
			}
			
			else if ((angle >= 22.5 && angle <= 67.5) || (angle < -122.5 && angle >= -157.5))
			{
				if (result.at<uchar>(y, x) >= result.at<uchar>(y + 1, x + 1) &&
					result.at<uchar>(y, x) >= result.at<uchar>(y - 1, x - 1))
				{
					thinning.at<uchar>(y, x) = result.at<uchar>(y, x);
				}
				else
					thinning.at<uchar>(y, x) = 0;
			}

			else if ((angle >= 67.5 && angle <= 112.5) || (angle < -67.5 && angle >= -112.5))
			{
				if (result.at<uchar>(y, x) >= result.at<uchar>(y, x + 1) &&
					result.at<uchar>(y, x) >= result.at<uchar>(y, x - 1))
				{
					thinning.at<uchar>(y, x) = result.at<uchar>(y, x);
				}
				else
					thinning.at<uchar>(y, x) = 0;
			}

			else if ((angle >= 112.5 && angle <= 157.5) || (angle < -22.5 && angle >= -67.5))
			{
				if (result.at<uchar>(y, x) >= result.at<uchar>(y - 1, x + 1) &&
					result.at<uchar>(y, x) >= result.at<uchar>(y + 1, x - 1))
				{
					thinning.at<uchar>(y, x) = result.at<uchar>(y, x);
				}
				else
					thinning.at<uchar>(y, x) = 0;
			}

		}
	const int t1 = 100;
	const int t2 = 100;

#endif
	cv::imshow("edgetihnnig", thinning);
	
	cv::Mat final_result;
	final_result = cv::Mat::zeros(origin.size(), origin.type());

	// thresholding
	std::vector<vec2> coords;

	// strong edges?
	for (int x = 1; x < width - 1; x++)
		for (int y = 1; y < height - 1; y++)
		{
			double value = thinning.at<uchar>(y, x);
			if (value >= t2)
				final_result.at<uchar>(y, x) = 255;
			else if (value >= t1 && value <= t2)
				coords.push_back({ (float)x, (float)y });
			else
				final_result.at<uchar>(y, x) = 0;
		}

	// weak edges
	for (vec2 point : coords)
	{
		// is any set pixel around this one 
		int amount = 0;
		for (int x = -1; x < 2; x++)
			for (int y = -1; y < 2; y++)
				if (final_result.at<uchar>(point.y + y, point.x + x) != 0)
					amount += 1;
		if (amount >= 1)
			final_result.at<uchar>(point.y, point.x) = 255;
		else
			final_result.at<uchar>(point.y, point.x) = 0;

	}



	cv::imshow("final_result", final_result);
	return final_result;

#if 0
	// double threshold
	const int t1 = 5;
	const int t2 = 5;

	for (int j = 1; j < width - 1; j++)
		for (int i = 1; i < height - 1; i++)
		{
			double val = (double)edge_buffer.at<uchar>(i, j);

			if (val >= t2)
			{
				final_buffer.at<uchar>(i, j) = 255;
			}
			else if (val >= t1 && val <= t2)
			{
				if ((edge_buffer.at<uchar>(i - 1, j) > t2) || (edge_buffer.at<uchar>(i + 1, j) > t2) || edge_buffer.at<uchar>(i, j - 1) > t2 || edge_buffer.at<uchar>(i, j + 1) > t2)
					final_buffer.at<uchar>(i, j) = 255;

			}
			else final_buffer.at<uchar>(i, j) = 0;
		
		}

#endif
	
}

struct two_channel_image
{
	cv::Mat real;
	cv::Mat comp;

};

enum filter_type
{
	LOW = 0, HIGH
};

two_channel_image dft2(cv::Mat origin)
{
	//cv::Mat origin_gray;
	//cv::cvtColor(origin, origin_gray, CV_BGR2GRAY);
	//cv::Mat origin_64b1c;
	//origin_gray.convertTo(origin_64b1c, CV_64FC1, 1.0 / 255.0);
	//cv::Mat result = cv::Mat(origin_gray.rows, origin_gray.cols, CV_64FC2);
	//cv::cvtColor(origin_gray, result, CV_8UC2);
	//cv::Mat result_64b1c = cv::Mat(origin.rows, origin.cols, CV_64FC1);
	//cv::Mat result_64b2c = cv::Mat(origin.rows, origin.cols, CV_64FC2);
	//result.convertTo(result_64b1c, CV_64FC1, 1.0 / 255.0);
	//result.convertTo(result_64b2c, CV_64FC2, 1.0 / 255.0);

	cv::Mat origin_DFT_;
	cv::Mat DFT_BGR2GRAY;
	cv::Mat DFT_BGR2GRAY_result;

	// complex
	cv::Mat real_part;
	cv::Mat comp_part;


	cv::cvtColor(origin, origin_DFT_, CV_BGR2GRAY);
	origin_DFT_.convertTo(DFT_BGR2GRAY, CV_64FC1, 1.0 / 255.0);
	origin_DFT_.convertTo(DFT_BGR2GRAY_result, CV_64FC1, 1.0 / 255.0);
	origin_DFT_.convertTo(real_part, CV_64FC1, 1.0 / 255.0);
	origin_DFT_.convertTo(comp_part, CV_64FC1, 1.0 / 255.0);

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
			
			double phase = atan(comp / real);
			double spec = sqrt(real * real + comp * comp);
			double power = log(spec);

			DFT_BGR2GRAY_result.at<double>(l, k) = power;

			real = real * scale;
			comp = comp * scale;

			real_part.at<double>(l, k) = real;
			comp_part.at<double>(l, k) = comp;

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
		Mat origin_up;
		int scale = 4;
		resize(DFT_BGR2GRAY_result, resized_up, Size(64 * scale, 64 * scale), INTER_LINEAR);
		resize(origin_DFT_, origin_up, Size(64 * scale, 64 * scale), INTER_LINEAR);

		imshow("DFT_origin", DFT_BGR2GRAY);
		imshow("origin_scaled", origin_up);
		imshow("result_scaled", resized_up);
		imshow("result", DFT_BGR2GRAY_result);


		imshow("real", real_part);
		imshow("comp", comp_part);
	}

	two_channel_image result__;
	result__.real = real_part;
	result__.comp = comp_part;

	return result__;

}
cv::Mat idft2(two_channel_image origin)
{
	cv::Mat result;
	origin.real.convertTo(result, CV_64FC1, 1.0 / 255.0);

	double rows = origin.real.rows;
	double cols = origin.real.cols;
	double scale = 1.0 / sqrt(rows * cols);


	for (int m = 0; m < rows; m++)
		for (int n = 0; n < cols; n++)
		{
			double sum = 0.0;

			for (int k = 0; k < rows; k++)
				for (int l = 0; l < cols; l++)
				{
					double real_origin = origin.real.at<double>(k, l);
					double comp_origin = origin.comp.at<double>(k, l);

					double argument = 2 * M_PI * (((m * k) / (double)rows) + ((n * l) / (double)cols));

					double real = cos(argument) * scale;
					double comp = sin(argument) * scale;

					sum += (real * real_origin) - (comp * comp_origin);
				}

			result.at<double>(m, n) = sum;
				
		}

	{
		using namespace std;
		using namespace cv;
		Mat resized_up;
		int scale = 4;
		resize(result, resized_up, Size(64 * scale, 64 * scale), INTER_LINEAR);

		cv::imshow("idft", result);
		cv::imshow("idft_resized", resized_up);
	}

	return result;
	
}

two_channel_image dft_apply_filter(two_channel_image origin, filter_type type, int radius)
{
	int rows = origin.real.rows;
	int cols = origin.real.cols;

	// generate mask
	cv::Mat mask;
	origin.real.convertTo(mask, CV_64FC1, 1.0 / 255.0);
	for (int x = 0; x < rows; x++)
		for (int y = 0; y < cols; y++)
			mask.at<double>(x, y) = 0.0;

	//cv::circle(mask, cv::Point(rows / 2, cols / 2), cv::Scalar(1.0), 0);
	cv::circle(mask, cv::Point(rows / 2, cols / 2), radius, cv::Scalar(1.0), -1);
	// change 1st and 3rd quadrant 
	// change 2nd and 4th quadrant
	for (int y = 0; y < cols; y++)
		for (int x = 0; x < rows; x++)
		{
			// 3rd
			if (x >= rows / 2 && y >= cols / 2)
			{
				double tmp = mask.at<double>(y, x);
				int x_ = x - rows / 2;
				int y_ = y - cols / 2;

				std::swap(mask.at<double>(y, x), mask.at<double>(y_, x_));
			}
			// 4th
			if (x >= rows / 2 && y < cols / 2)
			{
				double tmp = mask.at<double>(y, x);
				int x_ = x - rows / 2;
				int y_ = y + cols / 2;

				std::swap(mask.at<double>(y, x), mask.at<double>(y_, x_));
			}
		}



	cv::imshow("mask", mask);

	if (type == filter_type::LOW)
	{
		for (int x = 0; x < rows; x++)
			for (int y = 0; y < cols; y++)
			{
				if (mask.at<double>(x, y) < 0.5)
				{
					origin.real.at<double>(x, y) = 0.0;
					origin.comp.at<double>(x, y) = 0.0;
				}
			}
	}

	if (type == filter_type::HIGH)
	{
		for (int x = 0; x < rows; x++)
			for (int y = 0; y < cols; y++)
			{
				if (mask.at<double>(x, y) > 0.5)
				{
					origin.real.at<double>(x, y) = 0.0;
					origin.comp.at<double>(x, y) = 0.0;
				}
			}
	}

	return origin;
}

two_channel_image dft_apply_filter_bars(two_channel_image origin, filter_type type, int radius)
{
	int rows = origin.real.rows;
	int cols = origin.real.cols;

	// generate mask
	cv::Mat mask;
	origin.real.convertTo(mask, CV_64FC1, 1.0 / 255.0);
	for (int x = 0; x < rows; x++)
		for (int y = 0; y < cols; y++)
			mask.at<double>(x, y) = 1.0;

	cv::rectangle(mask, cv::Rect(0, (rows / 2) - (1 / 2), cols, 1), cv::Scalar(0), -1);
	cv::circle(mask, cv::Point(cols / 2, rows / 2), radius, cv::Scalar(1), -1);

	// change 1st and 3rd quadrant 
	// change 2nd and 4th quadrant
	for (int y = 0; y < cols; y++)
		for (int x = 0; x < rows; x++)
		{
			// 3rd
			if (x >= rows / 2 && y >= cols / 2)
			{
				double tmp = mask.at<double>(y, x);
				int x_ = x - rows / 2;
				int y_ = y - cols / 2;

				std::swap(mask.at<double>(y, x), mask.at<double>(y_, x_));
			}
			// 4th
			if (x >= rows / 2 && y < cols / 2)
			{
				double tmp = mask.at<double>(y, x);
				int x_ = x - rows / 2;
				int y_ = y + cols / 2;

				std::swap(mask.at<double>(y, x), mask.at<double>(y_, x_));
			}
		}



	cv::imshow("mask", mask);

	if (type == filter_type::LOW)
	{
		for (int x = 0; x < rows; x++)
			for (int y = 0; y < cols; y++)
			{
				if (mask.at<double>(x, y) == 0.0)
				{
					origin.real.at<double>(x, y) = 0.0;
					origin.comp.at<double>(x, y) = 0.0;
				}
			}
	}

	return origin;
}



int main()
{


#if GAMMA
	cv::Mat src_8uc3_moon = cv::imread("images/moon.jpg", CV_LOAD_IMAGE_COLOR);
	cv::Mat src_8uc3_moon_pow0_5 = cv::imread("images/moon.jpg", CV_LOAD_IMAGE_COLOR);
	cv::Mat src_8uc3_moon_pow2 = cv::imread("images/moon.jpg", CV_LOAD_IMAGE_COLOR);
	cv::imshow("original", src_8uc3_moon);

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

#if DFT2
	cv::Mat dft_src = cv::imread("images/lena64_noise.png", CV_LOAD_IMAGE_COLOR);
	two_channel_image result = dft2(dft_src);
#if LOW_PASS
	result = dft_apply_filter(result, filter_type::LOW, 20);
#endif
#if HIGH_PASS
	result = dft_apply_filter(result, filter_type::HIGH, 10);
#endif
	idft2(result);
#endif

#if FILTER_BARS
	cv::Mat dft_src_ = cv::imread("images/lena64_bars.png", CV_LOAD_IMAGE_COLOR);
	two_channel_image result_ = dft2(dft_src_);
	result_ = dft_apply_filter_bars(result_, filter_type::LOW, 2);
	idft2(result_);
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

#if EDGE_DETECTION
	cv::Mat valve = cv::imread("images/valve.png", CV_LOAD_IMAGE_COLOR);
	cv::Mat valve_gray;
	cv::cvtColor(valve, valve_gray, CV_BGR2GRAY);
	cv::Mat result = edge_detection(valve_gray);
	//edge_detection(result);
#endif

	cv::waitKey(0); // wait until keypressed

	return 0;
}
