#include "latitudeCorrection.h"

Mat latitudeCorrection(Mat imgOrg, Point2i center, int radius, double camerFieldAngle, CorrectType type)
{
	if (!(camerFieldAngle > 0 && camerFieldAngle <= PI))
	{
		cout << "The parameter \"camerFieldAngle\" must be in the interval (0,PI]." << endl;
		return Mat();
	}
	double rateOfWindow = 0.9;
	int width = imgOrg.size().width*rateOfWindow;
	int height = width;
	Size imgSize(width, height);

	Mat retImg(imgSize, CV_8UC3, Scalar(0, 0, 0));

	double dx = camerFieldAngle / imgSize.width;
	double dy = dx;

	//coordinate for latitude map
	double latitude;
	double longitude;

	//unity sphere coordinate 
	double x, y, z, r;

	//parameter cooradinate of sphere coordinate
	double Theta_sphere;
	double Phi_sphere;

	//polar cooradinate for fish-eye Image
	double p;
	double theta;

	//cartesian coordinate 
	double x_cart, y_cart;

	//Image cooradinate of imgOrg
	int u, v;

	//Image cooradinate of imgRet
	int u_latitude, v_latitude;

	//offset of imgRet Origin
	double longitude_offset, latitude_offset;
	longitude_offset = (PI - camerFieldAngle) / 2;
	latitude_offset = (PI - camerFieldAngle) / 2;

	Mat_<Vec3b> _retImg = retImg;
	Mat_<Vec3b> _imgOrg = imgOrg;

	//according to the correct type to do the calibration
	switch (type)
	{
	case Forward:
		int left, top;
		left = center.x - radius;
		top = center.y - radius;
		for (int j = top; j < top + 2 * radius; j++)
		{
			for (int i = left; i < left + 2 * radius; i++)
			{
				if (pow(i - center.x, 2) + pow(j - center.y, 2)>pow(radius, 2))
					continue;
				//Origin image cooradinate in pixel
				u = i;
				v = j;

				double R = radius / sin(camerFieldAngle / 2);

				//Convert to cartiesian cooradinate in unity circle
				x_cart = (u - center.x) / R;
				y_cart = -(v - center.y) / R;

				//convert to polar axes
				theta = cvFastArctan(y_cart, x_cart)*PI / 180;
				p = sqrt(pow(x_cart, 2) + pow(y_cart, 2));

				//convert to sphere surface parameter cooradinate
				Theta_sphere = asin(p);
				Phi_sphere = theta;

				//convert to sphere surface 3D cooradinate
				x = sin(Theta_sphere)*cos(Phi_sphere);
				y = sin(Theta_sphere)*sin(Phi_sphere);
				z = cos(Theta_sphere);

				//convert to latitiude  cooradinate
				latitude = acos(y);
				longitude = cvFastArctan(z, -x)*PI / 180;

				//transform the latitude to pixel cooradinate

				u_latitude = ((longitude - longitude_offset) / dx);
				v_latitude = ((latitude - latitude_offset) / dy);

				if (u_latitude < 0 || u_latitude >= imgSize.height || v_latitude < 0 || v_latitude >= imgSize.width)
					continue;

				//perform the map from the origin image to the latitude map image
				_retImg(v_latitude, u_latitude)[0] = _imgOrg(j, i)[0];
				_retImg(v_latitude, u_latitude)[1] = _imgOrg(j, i)[1];
				_retImg(v_latitude, u_latitude)[2] = _imgOrg(j, i)[2];
			}
		}

		break;

	case Reverse:

		for (int j = 0; j < imgSize.height; j++)
		{

			latitude = latitude_offset + j*dy;
			for (int i = 0; i < imgSize.width; i++)
			{

				longitude = longitude_offset + i*dx;
				//Convert from latitude cooradinate to the sphere cooradinate
				x = -sin(latitude)*cos(longitude);
				y = cos(latitude);
				z = sin(latitude)*sin(longitude);

				//Convert from sphere cooradinate to the parameter sphere cooradinate
				Theta_sphere = acos(z);
				Phi_sphere = cvFastArctan(y, x);//return value in Angle
				Phi_sphere = Phi_sphere*PI / 180;//Convert from Angle to Radian


				//Convert from parameter sphere cooradinate to fish-eye polar cooradinate
				p = sin(Theta_sphere);
				theta = Phi_sphere;

				//Convert from fish-eye polar cooradinate to cartesian cooradinate
				x_cart = p*cos(theta);
				y_cart = p*sin(theta);

				//double R = radius / sin(camerFieldAngle / 2);
				double R = radius;
				//Convert from cartesian cooradinate to image cooradinate
				u = x_cart*R + center.x;
				v = -y_cart*R + center.y;

				//if (pow(u - center.x, 2) + pow(v - center.y, 2) > pow(radius, 2))
				//{
				//	_imgOrg(v, u)[0] = 255;
				//	_imgOrg(v, u)[1] = 255;
				//	_imgOrg(v, u)[2] = 255;
				//	continue;
				//}

				_retImg.at<Vec3b>(j, i) = _imgOrg.at<Vec3b>(v,u);


			}
		}

		break;
	default:
		cout << "The CorrectType is Wrong! It should be \"Forward\" or \"Reverse\"." << endl;
		return Mat();
	}
	//imshow("org", _imgOrg);
	//imshow("ret", _retImg);
	//waitKey();
#ifdef _DEBUG_
	namedWindow("Corrected Image", CV_WINDOW_AUTOSIZE);
	imshow("Corrected Image", retImg );
	waitKey();
#endif
	return retImg;
}

Mat latitudeCorrection2(Mat imgOrg, Point2i center, int radius, distMapMode distMap, double camerFieldAngle,
	camMode camProjMode)
{
	if (!(camerFieldAngle > 0 && camerFieldAngle <= PI))
	{
		cout << "The parameter \"camerFieldAngle\" must be in the interval (0,PI]." << endl;
		return Mat();
	}
	double rateOfWindow = 0.9;

	//int width = imgOrg.size().width*rateOfWindow;
	//int height = width;

	int width = imgOrg.cols;
	int height = imgOrg.rows;


	Size imgSize(width, height);
	int center_x = imgSize.width / 2;
	int center_y = imgSize.height / 2;

	Mat retImg(imgSize, CV_8UC3, Scalar(0, 0, 0));

	double dx = camerFieldAngle / imgSize.width;
	double dy = camerFieldAngle / imgSize.height;

	//coordinate for latitude map
	double latitude;
	double longitude;

	//unity sphere coordinate 
	double x, y, z, r;

	//parameter cooradinate of sphere coordinate
	double Theta_sphere;
	double Phi_sphere;

	//polar cooradinate for fish-eye Image
	double p;
	double theta;

	//cartesian coordinate 
	double x_cart, y_cart;

	//Image cooradinate of imgOrg
	double u, v;
	Point pt,pt1, pt2, pt3, pt4;

	//Image cooradinate of imgRet
	int u_latitude, v_latitude;
	Rect imgArea(0, 0, imgOrg.cols, imgOrg.rows);

	//offset of imgRet Origin
	double longitude_offset, latitude_offset;
	longitude_offset = (PI - camerFieldAngle) / 2;
	latitude_offset = (PI - camerFieldAngle) / 2;

	double foval = 0.0;//焦距


	Mat_<Vec3b> _retImg = retImg;
	Mat_<Vec3b> _imgOrg = imgOrg;

	//according to the camera type to do the calibration
	for (int j = 0; j < imgSize.height; j++)
	{

		for (int i = 0; i < imgSize.width; i++)
		{
			Point3f tmpPt(i - center_x, center_y - j, 1600);//最后一个参数用来修改成像面的焦距
			double normPt = norm(tmpPt);

			switch (distMap)
			{
			case PERSPECTIVE:

				tmpPt.x /= normPt;
				tmpPt.y /= normPt;
				tmpPt.z /= normPt;

				x = tmpPt.x;
				y = tmpPt.y;
				z = tmpPt.z;

				break;
			case LATITUDE_LONGTITUDE:

				latitude = latitude_offset + j*dy;
				longitude = longitude_offset + i*dx;
				//Convert from latitude cooradinate to the sphere cooradinate
				x = -sin(latitude)*cos(longitude);
				y = cos(latitude);
				z = sin(latitude)*sin(longitude);

				break;
			default:
				break;
			}

			//Convert from unit sphere cooradinate to the parameter sphere cooradinate
			Theta_sphere = acos(z);
			Phi_sphere = cvFastArctan(y, x);//return value in Angle
			Phi_sphere = Phi_sphere*PI / 180;//Convert from Angle to Radian


			switch (camProjMode)
			{
			case STEREOGRAPHIC:
				foval = radius / (2 * tan(camerFieldAngle / 4));
				p = 2 * foval*tan(Theta_sphere/2);
				break;
			case EQUIDISTANCE:
				foval = radius / (camerFieldAngle / 2);
				p = foval*Theta_sphere;
				break;
			case EQUISOLID:
				foval = radius / (2 * sin(camerFieldAngle / 4));
				p = 2 * foval*sin(Theta_sphere / 2);
				break;
			case ORTHOGONAL:
				foval = radius / sin(camerFieldAngle / 2);
				p = foval*sin(Theta_sphere);
				break;
			default:
				cout << "The camera mode hasn't been choose!" << endl;
			}
			//Convert from parameter sphere cooradinate to fish-eye polar cooradinate
			//p = sin(Theta_sphere);
			theta = Phi_sphere;

			//Convert from fish-eye polar cooradinate to cartesian cooradinate
			x_cart = p*cos(theta);
			y_cart = p*sin(theta);

			//double R = radius / sin(camerFieldAngle / 2);

			//Convert from cartesian cooradinate to image cooradinate
			u = x_cart + center.x;
			v = -y_cart + center.y;

			pt = Point(u, v);
			
			if (!pt.inside(imgArea))
			{
				continue;
			}
		
			_retImg.at<Vec3b>(j, i) = _imgOrg.at<Vec3b>(pt);

		}
	}

	//imshow("org", _imgOrg);
	//imshow("ret", _retImg);
	//waitKey();
#ifdef _DEBUG_
	namedWindow("Corrected Image", CV_WINDOW_AUTOSIZE);
	imshow("Corrected Image", retImg);
	waitKey();
#endif
	imwrite("ret.jpg", retImg);
	return retImg;
}
