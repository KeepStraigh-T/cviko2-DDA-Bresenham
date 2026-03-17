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

	if(!vertices.isEmpty())
		vertices.clear();

	if (!transformedVert.isEmpty())
		transformedVert.clear();

	drawActivated = false;

	update();
}

void ViewerWidget::clearVertices()
{
	if(vertices.size() > 0)
		vertices.clear();
}

void ViewerWidget::push_backVertex(QPoint point)
{
	vertices.push_back(point);
}

QPoint ViewerWidget::backVertex()
{
	if (vertices.size() > 0)
		return vertices.back();
	else
		return QPoint(0, 0);
}

QPoint ViewerWidget::firstVertex()
{
	if (vertices.size() > 0)
		return vertices.front();
	else
		return QPoint(0, 0);
}

qsizetype ViewerWidget::sizeVertex() { return vertices.size(); }


void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color)
{
	if((start.x() == end.x()) && (start.y() == end.y()))					// dx/dy == 0/0
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
			swapPoints(start, end);

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
	else																											// dy > dx -> m > 1 or m < -1
	{
		if(start.y() > end.y())																	// y0 > y1
			swapPoints(start, end);

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
			y--;
			twoY -= 2;
		}
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

void ViewerWidget::swapPoints(QPoint& start, QPoint& end)
{
	QPoint temp = start;
	start = end;
	end = temp;
}

//Transformations
void ViewerWidget::rotate(double angle, QColor color, int algType)
{
	if(!img)
		return;

	img->fill(Qt::white);

	double rad = angle * M_PI / 180.0;

	for(qsizetype i = 1; i < transformedVert.size(); i++)
	{
		int x = (transformedVert[i].x() - transformedVert[0].x()) * cos(rad) - (transformedVert[i].y() - transformedVert[0].y()) * sin(rad) + transformedVert[0].x() + 0.5;
		int y = (transformedVert[i].x() - transformedVert[0].x()) * sin(rad) + (transformedVert[i].y() - transformedVert[0].y()) * cos(rad) + transformedVert[0].y() + 0.5;
		transformedVert[i].setX(x);
		transformedVert[i].setY(y);

		drawLine(transformedVert[i - 1], transformedVert[i], color, algType);
	}

	drawLine(transformedVert.back(), transformedVert.front(), color, algType);
}

void ViewerWidget::scale(double factorX, double factorY, QColor color, int algType)
{
	if(isEmpty())
		return;

	img->fill(Qt::white);

	for(qsizetype i = 1; i < transformedVert.size(); i++)
	{
		QPoint ver;
		int x = (transformedVert[i].x() - transformedVert[0].x()) * factorX + transformedVert[0].x() + 0.5;
		int y = (transformedVert[i].y() - transformedVert[0].y()) * factorY + transformedVert[0].y() + 0.5;
		transformedVert[i].setX(x);
		transformedVert[i].setY(y);

		drawLine(transformedVert[i - 1], transformedVert[i], color, algType);
	}
	drawLine(transformedVert.back(), transformedVert.front(), color, algType);
}

void ViewerWidget::shear(double factorX, QColor color, int algType)
{
	if(isEmpty())
		return;

	img->fill(Qt::white);

	for(qsizetype i = 1; i < vertices.size(); i++)
	{
		QPoint vertx;
		int x = (transformedVert[i].x() - transformedVert[0].x()) + factorX * transformedVert[i].y() + transformedVert[0].x() + 0.5;
		transformedVert[i].setX(x);
		transformedVert[i].setY(transformedVert[i].y());
		drawLine(transformedVert[i - 1], transformedVert[i], color, algType);
	}
	drawLine(transformedVert.back(), transformedVert.front(), color, algType);
}

void ViewerWidget::symmetry(QColor color, int algType)
{
	if(isEmpty())
		return;

	img->fill(Qt::white);

	int lineVetricesAmount = 2;
	//transformedVert[0] = transformedVert[0];
	if(transformedVert.size() > lineVetricesAmount) // symmetry of polygon
	{
		drawLine(transformedVert[0], transformedVert[1], color, algType);

		int axisVektorX = transformedVert[1].x() - transformedVert[0].x();    //  = b = -u
		int axisVektorY = transformedVert[1].y() - transformedVert[0].y();    //  = a = v
		int a = axisVektorY;	// x value of normal
		int b = -axisVektorX;	// y value of normal
		int c = -a * transformedVert[0].x() - b * transformedVert[0].y();

		for(qsizetype polyVertexIdx = 2; polyVertexIdx < transformedVert.size(); polyVertexIdx++)
		{
			int x = transformedVert[polyVertexIdx].x() - 2 * a * ( (a * transformedVert[polyVertexIdx].x() + b * transformedVert[polyVertexIdx].y() + c) / (double)(a * a + b * b) ) + 0.5;
			int y = transformedVert[polyVertexIdx].y() - 2 * b * ( (a * transformedVert[polyVertexIdx].x() + b * transformedVert[polyVertexIdx].y() + c) / (double)(a * a + b * b) ) + 0.5;
			transformedVert[polyVertexIdx].setX(x);
			transformedVert[polyVertexIdx].setY(y);

			// connect current modified and previous vertices
			drawLine(transformedVert[polyVertexIdx - 1], transformedVert[polyVertexIdx], color, algType);
		}
		drawLine(transformedVert.front(), transformedVert.back(), color, algType); // connect first and last vertices
	}
	else if(transformedVert.size() == lineVetricesAmount) // symmetry of line segment
	{
		// symmetry axis is parallel to Ox
		if(qAbs(transformedVert.back().x() - transformedVert.front().x()) > qAbs(transformedVert.back().y() - transformedVert.front().y()))				// dy < dx
		{
			int axisVektorX = img->width();
			int b = -axisVektorX;	// y value of normal
			int c = -b * transformedVert[0].y();

			int y = transformedVert[1].y() - 2 * b * ((b * transformedVert[1].y() + c) / (double) (b * b)) + 0.5;
			transformedVert[1].setY(y);

			drawLine(transformedVert.front(), transformedVert.back(), color, algType);
		}
		// symmetry axis is parallel to Oy
		else
		{
			int axisVektorY = img->height();
			int a = axisVektorY;
			int c = -a * transformedVert[0].x();

			int x = transformedVert[1].x() - 2 * a * ((a * transformedVert[1].x() + c) / (double) (a * a)) + 0.5;
			transformedVert[1].setX(x);

			drawLine(transformedVert.front(), transformedVert.back(), color, algType);
		}
	}
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}

