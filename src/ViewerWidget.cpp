#include "ViewerWidget.h"
#include <iostream>
#include <algorithm>
#include <cmath>

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if(imgSize != QSize(0, 0))
	{
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

/* Algorithms */
// Line rasterization
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
// Circle rasterization
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
void ViewerWidget::drawCircle(QColor color)
{
	if(!img || !data) return;

	drawCircleBresenham(vertices[0], vertices[1], color);
	update();
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
// Clipping
QVector<QPoint> ViewerWidget::clippingPolygon()
{
	QVector<QPoint> tempPoints = transformedVert;
	QVector<QPoint> clippedPoints;

	int xMin[]{0, -(img->height() - 1), -(img->width() - 1), 0}; // need to decrement - axis goes from 0 to height/weight - 1)

	int turns = 0;
	while(turns < 4)
	{
		if(tempPoints.isEmpty()) // whole polygone outside clipping area
			break;

		QPoint S(tempPoints.back());
		for(qsizetype i = 0; i < tempPoints.size(); i++)
		{
			QPoint V = tempPoints[i];

			if(V.x() >= xMin[turns])
			{
				if(S.x() >= xMin[turns])
					clippedPoints.push_back(V);
				else
				{
					clippedPoints.push_back(QPoint(xMin[turns], S.y() + ((xMin[turns] - S.x()) / (double) (V.x() - S.x())) * (V.y() - S.y()) + 0.5));
					clippedPoints.push_back(V);
				}
			}
			else
				if(S.x() >= xMin[turns])
					clippedPoints.push_back(QPoint(xMin[turns], S.y() + ((xMin[turns] - S.x()) / (double) (V.x() - S.x())) * (V.y() - S.y()) + 0.5));

			S = V;
		}
		tempPoints = clippedPoints;
		clippedPoints.clear();
		for(qsizetype i = 0; i < tempPoints.size(); i++)
		{
			int xRotated = -tempPoints[i].y();
			int yRotated = tempPoints[i].x();
			tempPoints[i].setX(xRotated);
			tempPoints[i].setY(yRotated);
		}
		turns++;
	}

	return tempPoints;
}
QVector<QPoint> ViewerWidget::clippingLine()
{
	QVector<QPoint> viewerWidgetEdges;
	viewerWidgetEdges.push_back(QPoint(0, 0));
	viewerWidgetEdges.push_back(QPoint(0, img->height()));
	viewerWidgetEdges.push_back(QPoint(img->width(), img->height()));
	viewerWidgetEdges.push_back(QPoint(img->width(), 0));

	double tL = 0.0;
	double tU = 1.0;
	QPoint d(transformedVert[1].x() - transformedVert[0].x(), transformedVert[1].y() - transformedVert[0].y());

	qsizetype i = 0;
	while(i < viewerWidgetEdges.size())
	{
		QPoint normal;
		if(i == viewerWidgetEdges.size() - 1) // upper edge
		{
			normal.setX(viewerWidgetEdges.back().y() - viewerWidgetEdges.front().y());
			normal.setY(-(viewerWidgetEdges.back().x() - viewerWidgetEdges.front().x()));
		}
		else // other edges
		{
			normal.setX(viewerWidgetEdges[i + 1].y() - viewerWidgetEdges[i].y());
			normal.setY(-(viewerWidgetEdges[i + 1].x() - viewerWidgetEdges[i].x()));
		}
		QPoint w(transformedVert[0].x() - viewerWidgetEdges[i].x(), transformedVert[0].y() - viewerWidgetEdges[i].y());

		int dn = d.x() * normal.x() + d.y() * normal.y();
		int wn = w.x() * normal.x() + w.y() * normal.y();

		double t = -wn / (double) dn;

		if(dn != 0)
		{
			if(dn > 0 && t <= 1)
				tL = qMax(t, tL);
			else if(dn < 0 && t >= 0)
				tU = qMax(t, tU);
		}
		i++;
	}

	if(tL == 0.0 && tU == 1.0)
	{
		if(!(transformedVert[0].x() <= img->width() && transformedVert[0].x() >= 0 && transformedVert[0].y() <= img->height() && transformedVert[0].y() >= 0))
			return transformedVert;
	}
	else if(tL > 0 && tL < 1 && tU > 0 && tU < 1)
	{
		QVector<QPoint> clippedPoints;
		clippedPoints.push_back(QPoint(transformedVert[0].x() + (transformedVert[1].x() - transformedVert[0].x()) * tL + 0.5, transformedVert[0].y() + (transformedVert[1].y() - transformedVert[0].y()) * tL + 0.5));
		clippedPoints.push_back(QPoint(transformedVert[0].x() + (transformedVert[1].x() - transformedVert[0].x()) * tU + 0.5, transformedVert[0].y() + (transformedVert[1].y() - transformedVert[0].y()) * tU + 0.5));
		return clippedPoints;
	}

	return transformedVert;
}
// Filling
void ViewerWidget::scanLine(const QVector <QPoint>& nodes, const QColor& color)
{
	if(nodes.isEmpty())
		return;

	struct Edge
	{
		int xz, yz;
		int yk;
		double w;
		bool operator<(const Edge& e){
			return yz < e.yz;
		}
	};

	int ymin, ymax{};

	// create list of edges based on polygon vertices (orient the edge from top to down (swap edge's vertices))
	QList <Edge> edges;																																						// polygon's edges
	Edge e{};
	for(qsizetype current = 0; current < nodes.size(); current++)																	// fill up list of edges of the polygon
	{
		qsizetype next = (current + 1) % nodes.size();

		if(nodes[current].y() == nodes[next].y())																										// skip horizontal edge
		{
			ymax = nodes[current].y() > ymax ? nodes[current].y() : ymax;
			continue;
		}
		else if(nodes[current].y() > nodes[next].y())																								// orient the edge from top to down (swap edge's vertices)
		{
			e.yz = nodes[next].y();
			e.xz = nodes[next].x();
			int xk = nodes[current].x();
			e.w = (e.xz - xk) / (double) (nodes[current].y() - nodes[next].y());												// 1 / m (m is slope)
			e.yk = nodes[current].y();
		}
		else
		{
			e.yz = nodes[current].y();
			e.xz = nodes[current].x();
			int xk = nodes[next].x();
			e.w = (xk - e.xz) / (double) (nodes[next].y() - nodes[current].y());												// 1 / m (m is slope)
			e.yk = nodes[next].y();
		}
		ymax = e.yk > ymax ? e.yk : ymax;
		e.w = (nodes[next].x() - nodes[current].x()) / (double) (nodes[next].y() - nodes[current].y());												// 1 / m (m is slope)
		e.yk -= 1;

		edges.append(e);
	}
#ifdef DEBUUG
	for(const Edge& edge : edges)
	{
		qDebug() << QString("(xz: %1, yz: %2, yk: %3)").arg(edge.xz).arg(edge.yz).arg(edge.yk);
	}
	qDebug() << "\n";
#endif

	if(edges.isEmpty())
		return;

	std::sort(edges.begin(), edges.end());																													// sort list so e[i].yz <= e[i+1].yz
#ifdef DEBUUG
	for(const Edge& edge : edges)
	{
		qDebug() << QString("(xz: %1, yz: %2, yk: %3)").arg(edge.xz).arg(edge.yz).arg(edge.yk);
	}
	qDebug() << "\n";
#endif
	ymin = edges[0].yz;

	struct scanLineEdge
	{
		int dy;																																												// number of rows which contain an edge
		double x;																																											// x coordinate of current intercept with current row
		double w;																																											// increment of x for each row
	};

	QVector <QList <scanLineEdge>> edgesTable{};
	edgesTable.resize(ymax - ymin + 1);

	// fill every row (QList) in table(QVector) with edges that have yz in this row
	for(qsizetype j = 0; j < edges.size(); j++)
	{
		scanLineEdge se;
		se.dy = edges[j].yk - edges[j].yz;
		se.x = edges[j].xz;
		se.w = edges[j].w;

		edgesTable[edges[j].yz - ymin].append(se);
	}

	QList <scanLineEdge> activeEdges{};

	int y{ymin};

	for(qsizetype i = 0; i < edgesTable.size(); i++)
	{
		activeEdges.append(edgesTable[i]);																																				// add new edges from row to activeEdges to if they start in this row
#ifdef DEBUUG
		for(const scanLineEdge& edge : activeEdges)
		{
			qDebug() << QString("x: %1, dy : %2, w : %3").arg(edge.x).arg(edge.dy).arg(edge.w);
		}
		qDebug() << "\n";
#endif
		std::sort(activeEdges.begin(), activeEdges.end(), [](const scanLineEdge& e1, const scanLineEdge& e2)			// sort edges in activeEdges 
 {
	 return e1.x < e2.x;
 });

#ifdef DEBUUG
		for(const scanLineEdge& edge : activeEdges)
		{
			qDebug() << QString("x: %1, dy : %2, w : %3").arg(edge.x).arg(edge.dy).arg(edge.w);
		}
		qDebug() << "\n";
#endif
		for(qsizetype j = 0; j + 1 < activeEdges.size(); j += 2)
		{
			int xCeil{qCeil(activeEdges[j].x)};
			int xFloor{qFloor(activeEdges[j + 1].x)};
			if(xCeil <= xFloor)
			{
				int x{xCeil};
				while(x <= xFloor)
					setPixel(x++, y, color);
			}
		}

		for(qsizetype i = 0; i < activeEdges.size(); i++)
		{
			activeEdges[i].dy -= 1;
			activeEdges[i].x += activeEdges[i].w;
		}

		for(qsizetype i = activeEdges.size() - 1; i >= 0; i--)
		{
			if(activeEdges[i].dy < 0)
				activeEdges.remove(i);
		}

		y++;
	}
}
void ViewerWidget::scanLineTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType)
{
	// 1. sort by y (ascending)
	if(p1.y() < p0.y()) std::swap(p0, p1);
	if(p2.y() < p0.y()) std::swap(p0, p2);
	if(p2.y() < p1.y()) std::swap(p1, p2);

	// 2. now: p0.y <= p1.y <= p2.y

	// handle special cases (aka sort by x (ascending))
	if(p1.y() == p2.y())
	{
		fillTopTriangle(p0, p1, p2, color, interpType, p0, p1, p2);
	}
	else if(p0.y() == p1.y())
	{
		fillBottomTriangle(p0, p1, p2, color, interpType, p0, p1, p2);
	}
	else
	{
		// 3. split triangle

		double alpha = (double) (p2.x() - p0.x()) / (p2.y() - p0.y());

		int px = p0.x() + alpha * (p1.y() - p0.y()) + 0.5;
		QPoint P(px, p1.y());

		// Decide left/right
		if(p1.x() < P.x())
		{
			fillTopTriangle(p0, p1, P, color, interpType, p0, p1, p2);
			fillBottomTriangle(p1, P, p2, color, interpType, p0, p1, p2);
		}
		else
		{
			fillTopTriangle(p0, P, p1, color, interpType, p0, p1, p2);
			fillBottomTriangle(P, p1, p2, color, interpType, p0, p1, p2);
		}
	}
}
void ViewerWidget::fillTopTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType, QPoint t0, QPoint t1, QPoint t2)								//p0 -> p1/P -> P/p1
{
	double w1 = (p1.x() - p0.x()) / (double) (p1.y() - p0.y());
	double w2 = (p2.x() - p0.x()) / (double) (p2.y() - p0.y());

	int y = p0.y();
	int ymax = p2.y();								// same as p1.y()
	double x1 = p0.x();
	double x2 = p0.x();
	
	// do not need to compare x1 and x2 for minority (as in second function for bottom triangle) beacause x1 and x2 start in one point (vertex p0)

	if(interpType == 1)
	{
		while(y < ymax)
		{
			int xStart = (int) (x1 + 1.0);									// ceil number
			int xEnd = (int) x2;														// floor number
			while(xStart <= xEnd)
			{
				setPixel(xStart, y, nearestNeighbor(xStart, y, t0, t1, t2));
				xStart++;
			}
			x1 += w1;
			x2 += w2;
			y++;
		}
	}
	else if(interpType == 2)
	{
		while(y < ymax)
		{
			int xStart = (int) (x1 + 1.0);									// ceil number
			int xEnd = (int) x2;														// floor number
			while(xStart <= xEnd)
			{
				setPixel(xStart, y, barycentricInterp(xStart, y, t0, t1, t2));
				xStart++;
			}
			x1 += w1;
			x2 += w2;
			y++;
		}
	}
	else
	{
		while(y < ymax)
		{
			int xStart = (int) (x1 + 1.0);									// ceil number
			int xEnd = (int) x2;														// floor number
			while(xStart <= xEnd)
			{
				setPixel(xStart, y, color);
				xStart++;
			}
			x1 += w1;
			x2 += w2;
			y++;
		}
	}
}

void ViewerWidget::fillBottomTriangle(QPoint p0, QPoint p1, QPoint p2, const QColor& color, int interpType, QPoint t0, QPoint t1, QPoint t2)						//P/p1 -> p1/P -> p2
{
	double w1 = (p2.x() - p0.x()) / (double) (p2.y() - p0.y());
	double w2 = (p2.x() - p1.x()) / (double) (p2.y() - p1.y());

	int y = p0.y();										// same as p1.y()
	int ymax = p2.y();
	double x1 = p0.x();
	double x2 = p1.x();

	if(x1 > x2)															//		p0			p1
		std::swap(x1, x2);										//				p2

	if(interpType == 1)
	{
		while(y < ymax)
		{
			int xStart = (int) (x1 + 1.0);									// ceil number
			int xEnd = (int) x2;														// floor number
			while(xStart <= xEnd)
			{
				setPixel(xStart, y, nearestNeighbor(xStart, y, t0, t1, t2));
				xStart++;
			}
			x1 += w1;
			x2 += w2;
			y++;
		}
	}
	else if(interpType == 2)
	{
		while(y < ymax)
		{
			int xStart = (int) (x1 + 1.0);									// ceil number
			int xEnd = (int) x2;														// floor number
			while(xStart <= xEnd)
			{
				setPixel(xStart, y, barycentricInterp(xStart, y, t0, t1, t2));
				xStart++;
			}
			x1 += w1;
			x2 += w2;
			y++;
		}
	}
	else
	{
		while(y < ymax)
		{
			int xStart = (int) (x1 + 1.0);									// ceil number
			int xEnd = (int) x2;														// floor number
			while(xStart <= xEnd)
			{
				setPixel(xStart, y, color);
				xStart++;
			}
			x1 += w1;
			x2 += w2;
			y++;
		}
	}
}
QColor ViewerWidget::nearestNeighbor(int x, int y, const QPoint& t0, const QPoint& t1, const QPoint& t2)
{
	int dx0 = x - t0.x();
	int dy0 = y - t0.y();
	int d0 = dx0 * dx0 + dy0 * dy0;

	int dx1 = x - t1.x();
	int dy1 = y - t1.y();
	int d1 = dx1 * dx1 + dy1 * dy1;

	int dx2 = x - t2.x();
	int dy2 = y - t2.y();
	int d2 = dx2 * dx2 + dy2 * dy2;

	int min = d0;
	int idx = 0;

	if(d1 < min)
	{
		min = d1; idx = 1;
	}
	if(d2 < min)
		idx = 2;

	static const QColor c0 = Qt::green;
	static const QColor c1 = Qt::red;
	static const QColor c2 = Qt::blue;

	return (idx == 0) ? c0 : (idx == 1) ? c1 : c2;
}
QColor ViewerWidget::barycentricInterp(int x, int y, const QPoint& t0, const QPoint& t1, const QPoint& t2)
{
	// celkova plocha trojuholnika T0,T1,T2
	double A = abs((t1.x() - t0.x()) * (t2.y() - t0.y()) - (t1.y() - t0.y()) * (t2.x() - t0.x())) / 2.0; // half magnitude of normal

	// plochy podtrojuholnikov s bodom P(x,y)
	double A0 = abs((t1.x() - x) * (t2.y() - y) - (t1.y() - y) * (t2.x() - x)) / 2.0;
	double A1 = abs((t0.x() - x) * (t2.y() - y) - (t0.y() - y) * (t2.x() - x)) / 2.0;
	double A2 = A - A0 - A1; // tretia plocha (aby sme nemuseli pocitat znova)

	// vahy (barycentricke suradnice)
	double l0 = A0 / A;
	double l1 = A1 / A;
	double l2 = 1.0 - l0 - l1;

	static QColor c0{QColor("yellow")};
	static QColor c1{QColor("purple")};
	static QColor c2{QColor("brown")};

	// interpolacia farby
	int r = (l0 * c0.red() + l1 * c1.red() + l2 * c2.red()) + 0.5;
	int g = (l0 * c0.green() + l1 * c1.green() + l2 * c2.green()) + 0.5;
	int b = (l0 * c0.blue() + l1 * c1.blue() + l2 * c2.blue() + 0.5);

	// orezanie na rozsah 0-255
	r = (r >= 255) ? 255 : (r < 0 ? 0 : r);
	g = (g >= 255) ? 255 : (g < 0 ? 0 : g);
	b = (b >= 255) ? 255 : (b < 0 ? 0 : b);

	return QColor(r, g, b);
}

void ViewerWidget::fergusovCubicCurve(QColor color, int algType)
{
	qsizetype pointAmount = curvePoints.size() - 1;
	double dt = 1.0 / (double)(pointAmount - 1);
	QPoint Q0{};
	QPoint Q1{};
	double t{};

	for (qsizetype i = 1; i < pointAmount; i++)
	{
		Q0 = curvePoints[i - 1].point;
		t = dt;
		while (t < 1)
		{
			double F0 = 2 * t * t * t - 3 * t * t + 1;
			double F1 = -2 * t * t * t + 3 * t * t;
			double F2 = t * t * t - 2 * t * t + t;
			double F3 = t * t * t - t * t;

			double x = curvePoints[i - 1].point.x() * F0 + curvePoints[i].point.x() * F1 + (curvePoints[i - 1].handle.x() - curvePoints[i - 1].point.x()) * F2 + (curvePoints[i].handle.x() - curvePoints[i].point.x()) * F3;
			double y = curvePoints[i - 1].point.y() * F0 + curvePoints[i].point.y() * F1 + (curvePoints[i - 1].handle.y() - curvePoints[i - 1].point.y()) * F2 + (curvePoints[i].handle.y() - curvePoints[i].point.y()) * F3;
			Q1.setX(x + 0.5);
			Q1.setY(y + 0.5);
			drawLine(Q0, QPoint(x + 0.5, y + 0.5), color, algType);
			Q0 = Q1;
			t += dt;
		}
		drawLine(Q0, curvePoints[i].point, color, algType);
	}

	for (qsizetype i = 0; i < pointAmount; i++)
	{
		drawLine(curvePoints[i].point, curvePoints[i].handle, color, algType);
	}
}

void ViewerWidget::bezierCurve(QColor color, int algType)
{
	QVector <QVector <QPointF>> points;
	points.resize(curvePoints.size());

	qsizetype pointAmount = curvePoints.size();

	for (qsizetype i = 0; i < pointAmount; i++)
		points[i].resize(pointAmount - i);

	for (qsizetype i = 0; i < pointAmount; i++)					// init first row with user entered points
	{
		points[0][i] = curvePoints[i].point;
	}

	double dt = 1.0 / (double)(pointAmount - 1);

	double t = dt;

	QPointF Q0{};
	QPointF Q1{};
	Q0 = points[0][0];

	while (t < 1.0)
	{
		for (qsizetype i = 1; i < pointAmount; i++)
		{
			for (qsizetype j = 0; j < pointAmount - i; j++)
			{
				points[i][j] = (1.0 - t) * points[i - 1][j] + t * points[i - 1][j + 1];
			}
		}

		Q1 = points[pointAmount - 1][0];
		drawLine(Q0.toPoint(), Q1.toPoint(), color, algType);
		Q0 = Q1;
		t += dt;

	}

	drawLine(Q0.toPoint(), curvePoints[pointAmount - 1].point, color, algType);

}

void ViewerWidget::coonsoveCubicBSpline(QColor color, int algType)
{
	if (curvePoints.size() < 4)												// algorithm work with n > 4 points
		return;

	double dt = 1.0 / (double)(pointAmount - 1);

	double B0 = [](double t) { return  (-1 / 6.0) * t * t * t + (1 / 2.0) * t * t - (1 / 2.0) * t + (1 / 6.0); };
	double B1 = [](double t) { return  (1 / 2.0) * t * t * t - t * t + (2 / 3.0); };
	double B2 = [](double t) { return  (-1 / 2.0) * t * t * t + (1 / 2.0) * t * t + (1 / 2.0) * t + 1 / 6.0; };
	double B3 = [](double t) { return  (1 / 6.0) * t * t * t; };

	QPointF Q0{};
	QPointF Q1{};
	for (qsizetype i = 3; i < curvePoints.size(); i++)
	{
		double t = 0.0;
		QPointF Q0 = curvePoints[i - 3].point * B0(0.0) + curvePoints[i - 2].point * B1(0.0) + curvePoints[i - 1].point * B2(0.0) + curvePoints[i].point * B3(0.0);
		while (t < 1.0)
		{
			t += dt;
			Q1 = curvePoints[i - 3].point * B0(t) + curvePoints[i - 2].point * B1(t) + curvePoints[i - 1].point * B2(t) + curvePoints[i].point * B3(t);

		}
	}

}
/* Algorithms */

/* Transformations */
void ViewerWidget::translation(QPoint currentMousePos)
{
	if(transformedVert.isEmpty())
		return;

	QPoint delta = currentMousePos - lastMousePos;

	for(QPoint& vertx : transformedVert)
	{
		vertx += delta;
		//qDebug() << QString("(%1, %2)").arg(vertx.x()).arg(vertx.y());
	}
	//qDebug() << "\n";

	lastMousePos = currentMousePos;
	update();
}
void ViewerWidget::rotate(double angle)
{
	double rad = angle * M_PI / 180.0;

	for(qsizetype i = 1; i < transformedVert.size(); i++)
	{
		int x = (transformedVert[i].x() - transformedVert[0].x()) * cos(rad) - (transformedVert[i].y() - transformedVert[0].y()) * sin(rad) + transformedVert[0].x() + 0.5;
		int y = (transformedVert[i].x() - transformedVert[0].x()) * sin(rad) + (transformedVert[i].y() - transformedVert[0].y()) * cos(rad) + transformedVert[0].y() + 0.5;
		transformedVert[i].setX(x);
		transformedVert[i].setY(y);
	}
}
void ViewerWidget::scale(double factorX, double factorY)
{
	if(transformedVert.isEmpty())
		return;

	for(qsizetype i = 1; i < transformedVert.size(); i++)
	{
		int x = (transformedVert[i].x() - transformedVert[0].x()) * factorX + transformedVert[0].x() + 0.5;
		int y = (transformedVert[i].y() - transformedVert[0].y()) * factorY + transformedVert[0].y() + 0.5;
		transformedVert[i].setX(x);
		transformedVert[i].setY(y);
	}
}
void ViewerWidget::shear(double factorX)
{
	if(transformedVert.isEmpty())
		return;

	for(qsizetype i = 1; i < vertices.size(); i++)
	{
		QPoint vertx;
		int x = (transformedVert[i].x() - transformedVert[0].x()) + factorX * transformedVert[i].y() + transformedVert[0].x() + 0.5;
		transformedVert[i].setX(x);
		transformedVert[i].setY(transformedVert[i].y());
	}
}
void ViewerWidget::symmetry()
{
	if(transformedVert.isEmpty())
		return;

	int lineVetricesAmount = 2;
	// Polygon symmetry
	if(transformedVert.size() > lineVetricesAmount)
	{
		int axisVektorX = transformedVert[1].x() - transformedVert[0].x();    //  = b = -u
		int axisVektorY = transformedVert[1].y() - transformedVert[0].y();    //  = a = v
		int a = axisVektorY;	// x value of normal
		int b = -axisVektorX;	// y value of normal
		int c = -a * transformedVert[0].x() - b * transformedVert[0].y();

		for(qsizetype polyVertexIdx = 2; polyVertexIdx < transformedVert.size(); polyVertexIdx++)
		{
			int x = transformedVert[polyVertexIdx].x() - 2 * a * ((a * transformedVert[polyVertexIdx].x() + b * transformedVert[polyVertexIdx].y() + c) / (double) (a * a + b * b)) + 0.5;
			int y = transformedVert[polyVertexIdx].y() - 2 * b * ((a * transformedVert[polyVertexIdx].x() + b * transformedVert[polyVertexIdx].y() + c) / (double) (a * a + b * b)) + 0.5;
			transformedVert[polyVertexIdx].setX(x);
			transformedVert[polyVertexIdx].setY(y);
		}
	}
	// Line symmetry
	else if(transformedVert.size() == lineVetricesAmount)
	{
		// symmetry axis is parallel to Ox
		if(qAbs(transformedVert.back().x() - transformedVert.front().x()) > qAbs(transformedVert.back().y() - transformedVert.front().y()))	// dy < dx
		{
			int axisVektorX = img->width();
			int b = -axisVektorX;	// y value of normal
			int c = -b * transformedVert[0].y();

			int y = transformedVert[1].y() - 2 * b * ((b * transformedVert[1].y() + c) / (double) (b * b)) + 0.5;
			transformedVert[1].setY(y);
		}
		// symmetry axis is parallel to Oy
		else
		{
			int axisVektorY = img->height();
			int a = axisVektorY;
			int c = -a * transformedVert[0].x();

			int x = transformedVert[1].x() - 2 * a * ((a * transformedVert[1].x() + c) / (double) (a * a)) + 0.5;
			transformedVert[1].setX(x);
		}
	}
}
/* Transformations */

// Drawing call functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if(!img || !data) return;

	if(algType == 0)
	{
		drawLineDDA(start, end, color);
	}
	else
	{
		drawLineBresenham(start, end, color);
	}
	update();
}
void ViewerWidget::drawPolygon(QColor color, int algType, int interpType)
{
	if(isEmpty())
		return;

	img->fill(Qt::white);

	if(transformedVert.size() > 2)																																	// Polygon
	{
		if(areaIsFilled)
		{
			if(transformedVert.size() == 3)																																																							// change this maybe because it'll fill clipped polygon(vertices > 3) too
				scanLineTriangle(transformedVert[0], transformedVert[1], transformedVert[2], color, interpType);																					// fill triangle
			else
				scanLine(transformedVert, color);																																																					// fill polygon (n > 3)
		}

		QVector <QPoint> clippedPoints = clippingPolygon();

		if(clippedPoints.size() > 1)																																	// whole/clipped polygon is inside clipping area
		{
			for(qsizetype i = 1; i < clippedPoints.size(); i++)
				drawLine(clippedPoints[i - 1], clippedPoints[i], color, algType);

			drawLine(clippedPoints.back(), clippedPoints[0], color, algType);
		}
	}
	else if(transformedVert.size() == 2)																															// line
	{
		QVector<QPoint> clippedPoints = clippingLine();
		if(clippedPoints.size() > 1)																																		// line must be clipped
			drawLine(clippedPoints[0], clippedPoints[1], color, algType);
	}
}
void ViewerWidget::drawCurve(QColor color, int curveType, int algType)
{
	if(isEmpty())
	return;

	img->fill(Qt::white);

	// leave approximated points on canvas
	for (qsizetype i = 1; i < curvePoints.size(); i++)
		setPixel(curvePoints[i].point.x(), curvePoints[i].point.y(), color);

	if(curveType == 0)																	// Hermite-Ferguson cubic
	{
		fergusovCubicCurve(color, algType);
	}
	else if (curveType == 1)
	{
		bezierCurve(color, algType);
	}
	else if (curveType == 2)
	{
		coonsoveCubicBSpline(color, algType);
	}
}

bool ViewerWidget::setImage(const QImage& inputImg)
{
	if(img)
	{
		delete img;
		img = nullptr;
		data = nullptr;
	}
	img = new QImage(inputImg.convertToFormat(QImage::Format_ARGB32));
	if(!img || img->isNull())
	{
		return false;
	}
	resizeWidget(img->size());
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if(img == nullptr)
	{
		return true;
	}

	if(img->size() == QSize(0, 0))
	{
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if(newSize != QSize(0, 0))
	{
		if(img != nullptr)
		{
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if(!img || img->isNull())
		{
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
		update();
	}

	return true;
}

// Set pixel
void ViewerWidget::setPixel(int x, int y, int r, int g, int b, int a)
{
	if(!img || !data) return;
	if(!isInside(x, y)) return;

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
	if(color.isValid())
	{
		setPixel(x, y, color.red(), color.green(), color.blue(), color.alpha());
	}
}

// Other
void ViewerWidget::swapPoints(QPoint& start, QPoint& end)
{
	QPoint temp = start;
	start = end;
	end = temp;
}
bool ViewerWidget::isInside(int x, int y)
{
	return img && x >= 0 && y >= 0 && x < img->width() && y < img->height();
}
void ViewerWidget::clear()
{
	if(!img) return;
	img->fill(Qt::white);

	if(!vertices.isEmpty())
		vertices.clear();

	if(!curvePoints.isEmpty())
		curvePoints.clear();

	if(!transformedVert.isEmpty())
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
	if(vertices.size() > 0)
		return vertices.back();
	else
		return QPoint(0, 0);
}
QPoint ViewerWidget::firstVertex()
{
	if(vertices.size() > 0)
		return vertices.front();
	else
		return QPoint(0, 0);
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if(!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}

