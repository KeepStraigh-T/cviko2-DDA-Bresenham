#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(600, 600), ui->scrollArea);
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

	globalColor = Qt::red;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
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
	{
		return false;
	}

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

	return QObject::eventFilter(obj, event);
}

void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

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
	{
		return;
	}

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
			msgBox.setText("Unable to save image.");
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

void ImageViewer::on_actionClear_triggered()
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


