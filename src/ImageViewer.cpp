#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);

	// Tabify 2 dock widgets
	tabifyDockWidget(ui->dockWidget_1, ui->dockWidget_2);
	ui->dockWidget_1->raise();
	ui->dockWidget_2->setWindowTitle(QString::fromUtf8("3D"));
	connect(this, &QMainWindow::tabifiedDockWidgetActivated, this, &ImageViewer::onTabifiedDockWidgetActivated);

	vW = new ViewerWidget(QSize(1500, 1000), ui->scrollArea);
	ui->scrollArea->setWidget(vW);

	QSizePolicy policy = ui->scrollArea->sizePolicy();
	policy.setVerticalStretch(1);
	policy.setHorizontalStretch(1);

	ui->scrollArea->setBackgroundRole(QPalette::Midlight);
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::darkBlue;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
}

ImageViewer::~ImageViewer()
{
	delete ui;
}

void ImageViewer::onTabifiedDockWidgetActivated(QDockWidget* dockWidget)
{
	if(dockWidget == ui->dockWidget_2)
		dock2IsVisible = true;
	else
		dock2IsVisible = false;
	
	vW->clear();
	uiAccessibility(true);
	vW->setDrawActivated(true);
	vW->setDragging(false);
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if(obj->objectName() == "ViewerWidget")
	{
		return ViewerWidgetEventFilter(obj, event);
	}
	return QMainWindow::eventFilter(obj, event);
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);
	if(!w)
		return false;

	if(event->type() == QEvent::MouseButtonPress)
	{
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if(event->type() == QEvent::MouseButtonRelease)
	{
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if(event->type() == QEvent::MouseMove)
	{
		ViewerWidgetMouseMove(w, event);
	}
	else if(event->type() == QEvent::Leave)
	{
		ViewerWidgetLeave(w, event);
	}
	else if(event->type() == QEvent::Enter)
	{
		ViewerWidgetEnter(w, event);
	}
	else if(event->type() == QEvent::Wheel)
	{
		ViewerWidgetWheel(w, event);
	}
	// standard event processing
	return QObject::eventFilter(obj, event);
}

void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if(dock2IsVisible) // 3D dockTab
		return;

	if(e->button() == Qt::LeftButton)
	{
		if(w->getDrawActivated())																															// start of drawing
		{
			uiAccessibility(false);																															// disable interface
			if(ui->comboBoxFigure->currentIndex() == 2)
			{
				QPoint point(e->pos().x(), e->pos().y());
				QPoint handle(point + QPoint(50, 0));
				w->curvePoints.push_back({point, handle});
			}
			else
				w->push_backVertex(e->pos());																												// add a vertex of polygon/line or circle

			w->setPixel(e->pos().x(), e->pos().y(), globalColor);																// set pixel of each vertex
			w->update();
		}
		else if(ui->comboBoxFigure->currentIndex() == 2 && ui->comboBoxCurveAlg->currentIndex() == 0)
		{
			QPoint pos = e->pos();
			const int radius = 50;																																	// radious of draggable area

			for(int i = 0; i < w->curvePoints.size(); i++)
			{
				// check main point
				if((w->curvePoints[i].point - pos).manhattanLength() < radius)
				{
					selectedIndex = i;
					selectedType = SelectedType::POINT;
					return;
				}

				// check handle
				else if((w->curvePoints[i].handle - pos).manhattanLength() < radius)
				{
					selectedIndex = i;
					selectedType = SelectedType::HANDLE;
					return;
				}
			}

			selectedType = SelectedType::NONE;
		}
		else if(!w->getDragging())																														// enable moving
		{
			w->setDragging(true);
			w->setLastMousePos(e->pos());
		}
		else																																									// disable moving
			w->setDragging(false);
	}

	else if(e->button() == Qt::RightButton && (w->sizeVertex() > 0 || w->curvePoints.size() > 0))
	{
		if(w->getDrawActivated())																															// first right click finishes drawing
		{
			if(ui->comboBoxFigure->currentIndex() == 0)																					// draw polygon
			{
				vW->initTransfVert();																															// initialize transformed vertices with original
				vW->drawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxInterpAlg->currentIndex());
				uiAccessibility(true);																															// unable interface
			}
			else if(ui->comboBoxFigure->currentIndex() == 1 && w->sizeVertex() == 2)						// draw circle
			{
				w->drawCircle(globalColor);
				uiAccessibility(true);																															// unable interface
			}
			else if(ui->comboBoxFigure->currentIndex() == 2 && w->curvePoints.size() >= 2)						// draw curve
			{
				vW->drawCurve(globalColor, ui->comboBoxCurveAlg->currentIndex(), ui->comboBoxLineAlg->currentIndex());
				uiAccessibility(true);																															// unable interface
			}
			w->setDrawActivated(false);
		}
		else																																									// second right click clears the canvas
		{
			clearCanvas();																																			// clear whole canvas on right button click (after finished drawing)
		}
	}
}

void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if(ui->comboBoxFigure->currentIndex() == 2)
	{
		if(selectedType == SelectedType::NONE || selectedIndex < 0)
			return;

		QPoint pos = e->pos();

		if(selectedType == SelectedType::POINT)
		{
			QPoint delta = pos - w->curvePoints[selectedIndex].point;

			// move point AND handle together
			w->curvePoints[selectedIndex].point = pos;
			w->curvePoints[selectedIndex].handle += delta;
		}
		else if(selectedType == SelectedType::HANDLE)
		{
			// move only handle
			w->curvePoints[selectedIndex].handle = pos;
		}

		update(); // redraw
		vW->drawCurve(globalColor, ui->comboBoxCurveAlg->currentIndex(), ui->comboBoxLineAlg->currentIndex());

		return;
	}

	if(!w->getDragging())
		return;

	w->translation(e->pos());

	if(ui->comboBoxFigure->currentIndex() == 0)
		vW->drawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxInterpAlg->currentIndex());
	else if(ui->comboBoxFigure->currentIndex() == 1 && vW->sizeVertex() == 2)
		vW->drawCircle(globalColor);
}

void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);

	int scaleFactorSign = wheelEvent->angleDelta().y();

	if(scaleFactorSign > 0)
		vW->scale(1.25, 1.25);
	else if(scaleFactorSign < 0)
		vW->scale(0.75, 0.75);

	vW->drawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxInterpAlg->currentIndex());
}

void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	selectedType = SelectedType::NONE;
	selectedIndex = -1;
}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if(QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if(!loadedImg.isNull())
	{
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if(fileName.isEmpty())
		return;

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if(!openImage(fileName))
	{
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if(!fileName.isEmpty())
	{
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if(!saveImage(fileName))
		{
			msgBox.setText("Unable to save vtk file.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else
		{
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}

void ImageViewer::on_pushButtonClear_clicked()
{
	clearCanvas();
}

void ImageViewer::clearCanvas()
{
	vW->setDragging(false);
	vW->setFilled(false);
	ui->pushButtonFill->setEnabled(false);
	ui->groupBox_3->setEnabled(false);
	if(ui->pushButtonFill->isChecked())
		ui->pushButtonFill->setChecked(false);
	vW->clear();
	vW->setDrawActivated(true);
}

void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if(newColor.isValid())
	{
		QString style_sheet = QString("background-color: %1;").arg(newColor.name(QColor::HexRgb));
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}

void ImageViewer::on_pushButtonRotate_clicked()
{
	if(vW->isEmpty() || vW->sizeVertex() == 0 || vW->getDrawActivated())
		return;

	vW->rotate(ui->rotateAngleSpinBox->value());
	vW->drawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxInterpAlg->currentIndex());
}

void ImageViewer::on_pushButtonScale_clicked()
{
	if(vW->isEmpty() || vW->sizeVertex() == 0 || vW->getDrawActivated())
		return;

	vW->scale(ui->xFactorScaleSpinBox->value(), ui->yFactorScaleSpinBox->value());
	vW->drawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxInterpAlg->currentIndex());
}

void ImageViewer::uiAccessibility(bool state)
{
	ui->pushButtonSetColor->setEnabled(state);
	ui->comboBoxInterpAlg->setEnabled(state);
	ui->pushButtonFill->setEnabled(state);
	ui->comboBoxFigure->setEnabled(state);
	ui->comboBoxLineAlg->setEnabled(state);
	ui->pushButtonClear->setEnabled(state);
	ui->actionClear->setEnabled(state);
	ui->groupBox_3->setEnabled(state);
}

void ImageViewer::on_pushButtonShear_clicked()
{
	if(vW->isEmpty() || vW->sizeVertex() == 0 || vW->getDrawActivated())
		return;

	vW->shear(ui->shearSpinBox->value());
	vW->drawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxInterpAlg->currentIndex());

}

void ImageViewer::on_pushButtonSymmetry_clicked()
{
	if(vW->isEmpty() || vW->sizeVertex() == 0 || vW->getDrawActivated())
		return;

	vW->symmetry();
	vW->drawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxInterpAlg->currentIndex());
}

void ImageViewer::on_comboBoxFigure_currentIndexChanged(int index)
{
	if(index == 1) // Circle in comboBoxFigure
		ui->comboBoxLineAlg->setEnabled(false);
	else
		ui->comboBoxLineAlg->setEnabled(true);
}

void ImageViewer::on_pushButtonFill_clicked()
{
	if(vW->isEmpty() || vW->sizeVertex() == 0 || vW->getDrawActivated())
		return;

	if(ui->pushButtonFill->isChecked())
		vW->setFilled(true);
	else
		vW->setFilled(false);

	vW->drawPolygon(globalColor, ui->comboBoxLineAlg->currentIndex(), ui->comboBoxInterpAlg->currentIndex());
}



void ImageViewer::on_actionLoad_vtk_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Vtk file (*vtk);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if(fileName.isEmpty())
		return;

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	QFile fd(fileName);
	if(!fd.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Error", "Could not open file for reading.");
		return;
	}

	QTextStream in(&fd);
	
	if(in.readLine() != "# vtk DataFile Version 3.0")
	{
		QMessageBox::critical(this, "Error", "Wrong file format");
		return;
	}

	// 1. Skip setup until we hit the POINTS declaration
	QString token;
	int numVertices = 0;
	while (!in.atEnd()) 
	{
		in >> token;
		if (token == "POINTS") 
		{
			in >> numVertices;			// Reads the number of vertices
			in >> token;
			break;
		}
	}

	// 2. Read exactly the number of vertices specified
	std::vector<Vertex> meshVertices;
	meshVertices.resize(numVertices);
	
	for (int i = 0; i < numVertices; ++i)
	{
		double x, y, z;
		in >> x >> y >> z;
		meshVertices[i] = Vertex(x, y, z);
	}
	scene.objMesh.setVertices(meshVertices);

	// 3. Find the POLYGONS section
	int numFaces = 0;
	int totalItems = 0;
	while (!in.atEnd())
	{
		in >> token;
		if (token == "POLYGONS")
		{
			in >> numFaces;   // Reads number of polygons
			in >> totalItems; // Reads total integers in list
			break;
		}
	}

	// 4. Read exactly the number of faces specified
	std::vector<std::array<idx_t, 3>> meshFaces;
	meshFaces.reserve(numFaces);

	for (int i = 0; i < numFaces; ++i)
	{
		int numPointsInFace;
		idx_t v1, v2, v3;
		 
		in >> numPointsInFace; // VTK format starts each polygon line with the vertex count (usually 3)
		in >> v1 >> v2 >> v3;

		meshFaces.push_back({v1, v2, v3});
	}
	scene.objMesh.setFaces(meshFaces);

	fd.close();
}

void ImageViewer::on_actionSave_vtk_triggered()
{
	if(scene.objMesh.getVertices().empty() || scene.objMesh.getFaces().empty())
		return;

	QString folder = settings.value("folder_img_save_path", "").toString();
	QString fileFilter = "Vtk file (*.vtk);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save vtk", folder, fileFilter);
	if(!fileName.isEmpty())
	{
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		QFile fd(fileName);
		if(!fd.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QMessageBox::critical(this, "Error", "Could not open file for writing.");
			return;
		}

		QTextStream out(&fd);

		out << "# vtk DataFile Version 3.0" << '\n';
		out << "vtk output" << '\n';
		out << "ASCII" << '\n';
		out << "DATASET POLYDATA" << '\n';
		out << "POINTS " << scene.objMesh.getVertices().size() << ' ' << "double" << '\n';

		const std::vector<Vertex> meshVertices = scene.objMesh.getVertices();
		for(idx_t i = 0; i < meshVertices.size(); i++)
			out << (int) meshVertices[i].x << ' ' << (int) meshVertices[i].y << ' ' << (int) meshVertices[i].z << '\n';

		const std::vector<std::array<idx_t, 3>> meshFaces = scene.objMesh.getFaces();
		size_t nFaces = meshFaces.size();
		out << "POLYGONS" << ' ' << nFaces << ' ' << nFaces * (meshFaces[0].size() + 1) << '\n';

		for(idx_t i = 0; i < nFaces; i++)
			out << meshFaces[0].size() << ' ' << meshFaces[i][0] << ' ' << meshFaces[i][1] << ' ' << meshFaces[i][2] << '\n';

		fd.close();
	}

	if(scene.objMesh.getVertices().empty() || scene.objMesh.getFaces().empty());
	return;
}

void ImageViewer::on_CB_Object_currentIndexChanged(int idx)
{
	if(idx == 0)
	{
		ui->labelCubeLength->show();
		ui->DSB_EdgeSize->show();
		ui->labelRadius->hide();
		ui->DSB_Radius->hide();
		ui->labelParallels->hide();
		ui->SB_Parallels->hide();
		ui->labelMeridians->hide();
		ui->SB_Meridians->hide();
	}
	else if(idx == 1)
	{
		ui->labelCubeLength->hide();
		ui->DSB_EdgeSize->hide();
		ui->labelRadius->show();
		ui->DSB_Radius->show();
		ui->labelParallels->show();
		ui->SB_Parallels->show();
		ui->labelMeridians->show();
		ui->SB_Meridians->show();
	}
}

void ImageViewer::renderScene()
{
	scene.cam.setCamera(ui->HSB_Zenit->value(), ui->HSB_Azimut->value(), ui->HS_Distance->value());
	scene.transformToCameraSpace();
	scene.project(ui->CB_ProjType->currentIndex(), ui->HS_Distance->value());

	const std::vector<std::array<idx_t, 3>>& faces = scene.objMesh.getFaces();
	int w = vW->getImage()->width();
	int h = vW->getImage()->height();
	// draw object
	for(size_t i = 0; i < faces.size(); i++) // 3 lines per triangle
	{
		QPoint p1(scene.sceneVertices[faces[i][0]].x + w/2.0 + 0.5, scene.sceneVertices[faces[i][0]].y + h/2.0 + 0.5);
		QPoint p2(scene.sceneVertices[faces[i][1]].x + w/2.0 + 0.5, scene.sceneVertices[faces[i][1]].y + h/2.0 + 0.5);
		QPoint p3(scene.sceneVertices[faces[i][2]].x + w/2.0 + 0.5, scene.sceneVertices[faces[i][2]].y + h/2.0 + 0.5);

		vW->drawLineBresenham(p1, p2, globalColor);
		vW->drawLineBresenham(p2, p3, globalColor);
		vW->drawLineBresenham(p3, p1, globalColor);
	}
}

void ImageViewer::on_PB_RenderObject_clicked()
{
	vW->clear();

	if(ui->CB_Object->currentText() == "Cube")
	{
		double cubeEdgeLen = ui->DSB_EdgeSize->value();
		if(cubeEdgeLen == 0.0)
		{
			QMessageBox::critical(this, "Error", "Please, enter the length of cube edge.");
			return;
		}
		scene.objMesh.buildCubeMesh(cubeEdgeLen);
	}
	else if(ui->CB_Object->currentText() == "Sphere")
	{
		if(ui->DSB_Radius->value() == 0.0)
		{
			QMessageBox::critical(this, "Error", "Please, enter the radius of sphere.");
			return;
		}
		// +1 because n paralles split sphere into n+1 horizontal segments
		scene.objMesh.buildUVSphereMesh(ui->DSB_Radius->value(), ui->SB_Parallels->value() + 1, ui->SB_Meridians->value());
	}

	renderScene();
}

void ImageViewer::on_HSB_Zenit_valueChanged(int newZenitVal)
{
	vW->clear();
	renderScene();
}

void ImageViewer::on_HSB_Azimut_valueChanged(int newAzimutVal)
{
	vW->clear();

	renderScene();
}

void ImageViewer::on_HS_Distance_valueChanged(int newDistance)
{
	vW->clear();

	renderScene();
}