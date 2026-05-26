#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"
#include "Mesh.h"

enum class SelectedType
{
	NONE,
	POINT,
	HANDLE
};

class ImageViewer: public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);
	~ImageViewer();
private:
	Ui::ImageViewerClass* ui = nullptr;
	ViewerWidget* vW = nullptr;


	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;
	//HalfEdgeMesh mesh;
	Mesh mesh;
	bool dock2IsVisible = false; // tab for 2D is selected

	int selectedIndex = -1;
	SelectedType selectedType = SelectedType::NONE;

	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

	void clearCanvas();
	void uiAccessibility(bool state);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionExit_triggered();
	void on_actionLoad_vtk_triggered();
	void on_actionSave_vtk_triggered();
	void on_comboBoxFigure_currentIndexChanged(int index);
	void on_pushButtonRotate_clicked();
	void on_pushButtonScale_clicked();
	void on_pushButtonShear_clicked();
	void on_pushButtonSymmetry_clicked();
	void on_pushButtonClear_clicked();
	void on_pushButtonFill_clicked();
	void onTabifiedDockWidgetActivated(QDockWidget* dockWidget);
	void on_PB_BuildCube_clicked();
	void on_PB_BuildSphere_clicked();

	//Tools slots
	void on_pushButtonSetColor_clicked();
};
