#include "ViewerWidget.h"
#include <iostream>
#include <algorithm>
#include <cmath>

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete img;
	img = nullptr;
	data = nullptr;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img) {
		delete img;
		img = nullptr;
		data = nullptr;
	}
	img = new QImage(inputImg.convertToFormat(QImage::Format_ARGB32));
	if (!img || img->isNull()) {
		return false;
	}
	resizeWidget(img->size());
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img || img->isNull()) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, int r, int g, int b, int a)
{
	if (!img || !data) return;
	if (!isInside(x, y)) return;

	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(b);
	data[startbyte + 1] = static_cast<uchar>(g);
	data[startbyte + 2] = static_cast<uchar>(r);
	data[startbyte + 3] = static_cast<uchar>(a);
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	setPixel(x, y, static_cast<int>(255 * valR + 0.5), static_cast<int>(255 * valG + 0.5), static_cast<int>(255 * valB + 0.5), static_cast<int>(255 * valA + 0.5));
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		setPixel(x, y, color.red(), color.green(), color.blue(), color.alpha());
	}
}

bool ViewerWidget::isInside(int x, int y)
{
	return img && x >= 0 && y >= 0 && x < img->width() && y < img->height();
}

//Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if (!img || !data) return;

	if (algType == 0) {
		drawLineDDA(start, end, color);
	}
	else {
		drawLineBresenham(start, end, color);
	}
	update();

	//Po implementovani drawLineDDA a drawLineBresenham treba vymazat
	//QPainter painter(img);
	//painter.setPen(QPen(color));
	//painter.drawLine(start, end);
	//update();
}

void ViewerWidget::drawCircle(QPoint center, QPoint end, QColor color)
{
	if (!img || !data) return;

	drawCircleBresenham(center, end, color);
	update();
}

void ViewerWidget::clear()
{
	if (!img) return;
	img->fill(Qt::white);
	update();
}

void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color)
{
	if((start.x() == end.x()) && (start.y() == end.y()))
		return;

	int dx = end.x() - start.x();
	int dy = end.y() - start.y();

	int steps = qMax(qAbs(dx), qAbs(dy));

	float xInc = dx / (float) steps;
	float yInc = dy / (float) steps;

	float x = start.x();
	float y = start.y();

	for(int idx = 0; idx < steps; idx++)
	{
		x += xInc;
		y += yInc;
		setPixel(x + 0.5f, y + 0.5f, color);
	}
}

void ViewerWidget::drawLineBresenham(QPoint start, QPoint end, QColor color)
{

	if(qAbs(end.y() - start.y()) < qAbs(end.x() - start.x()))			// dy < dx -> 0<m<1 or -1<m<0
	{
		if(start.x() > end.x())																			// x0 > x1
			ViewerWidget::swapPoints(start, end);

		int dx = end.x() - start.x();
		int dy = end.y() - start.y();

		int yInc = 1;
		if(dy < 0)
		{
			dy = -dy;
			yInc = -1;
		}

		int k1 = 2 * dy;
		int k2 = 2 * dy - 2 * dx;
		int p = 2 * dy - dx;

		int x = start.x();
		int y = start.y();

		while(x < end.x())
		{
			x += 1;

			if(p > 0)
			{
				y += yInc;
				p += k2;
			}
			else
				p += k1;

			setPixel(x, y, color);
		}
	}
	else
	{
		if(start.y() > end.y())																	// y0 > y1
			ViewerWidget::swapPoints(start, end);

		int dx = end.x() - start.x();
		int dy = end.y() - start.y();

		int xInc = 1;
		if(dx < 0)
		{
			dx = -dx;
			xInc = -1;
		}

		int k1 = 2 * dx;
		int k2 = 2 * dx - 2 * dy;
		int p = 2 * dx - dy;

		int x = start.x();
		int y = start.y();

		while(y < end.y())
		{
			y += 1;

			if(p > 0)
			{
				x += xInc;
				p += k2;
			}
			else
				p += k1;

			setPixel(x, y, color);
		}	
	}
}

void ViewerWidget::drawCircleBresenham(QPoint center, QPoint end, QColor color)
{
	int radius = std::sqrt(std::powl(end.x() - center.x(), 2) + std::pow(end.y() - center.y(), 2)) + 0.5f;
	int p = 1 - radius;

	int twoX = 3;
	int twoY = 2 * radius - 2;

	int x = 0;
	int y = radius;

	while(x <= y)
	{
		drawCirclePoints(center.x(), center.y(), x, y, color);

		if(p > 0)
		{
			p -= twoY;
			//p += 2 * x - 2 * y + 5;
			y--;
			twoY -= 2;
		}
		//else
			//p += 2 * x + 3;
		p += twoX;
		twoX += 2;
		x++;
	}
}

void ViewerWidget::drawCirclePoints(int xc, int yc, int x, int y, QColor color)
{
	setPixel(xc + x, yc + y, color);
	setPixel(xc - x, yc + y, color);
	setPixel(xc + x, yc - y, color);
	setPixel(xc - x, yc - y, color);
	setPixel(xc + y, yc + x, color);
	setPixel(xc - y, yc + x, color);
	setPixel(xc + y, yc - x, color);
	setPixel(xc - y, yc - x, color);
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}

void ViewerWidget::swapPoints(QPoint& start, QPoint& end)
{
	QPoint temp = start;
	start = end;
	end = temp;
}