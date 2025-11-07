#include "QtWidgetsApplication1.h"
#include "MInstTracer.h"

QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	short siInstId = 1;
	MInstTracer_InitTrace("Test", siInstId, 50);

	MInstTrtacer_FichierError(siInstId, "VES_INIT ***** VESLance ==> Pb InitBal() *****");

}

QtWidgetsApplication1::~QtWidgetsApplication1()
{}

