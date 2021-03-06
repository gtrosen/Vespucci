#include "plotviewer.h"
#include "ui_plotviewer.h"
#include "Global/global.h"
PlotViewer::PlotViewer(QWidget *parent, const arma::vec &x, const arma::vec &y, arma::vec z, QString x_label, QString y_label) :
    QDialog(parent),
    ui(new Ui::PlotViewer),
    context_menu_("Plot Options", this),
    directory_(QDir::homePath())
{
    ui->setupUi(this);
    plot_ = findChild<QCustomPlot *>("plot");
    plot_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowContextMenu(QPoint)));
    AddContextMenuItems();
    typedef std::vector<double> stdvec;
    arma::field<arma::vec> x_parts;
    arma::field<arma::vec> y_parts;
    arma::vec unique_z = arma::unique(z);
    x_parts.set_size(unique_z.n_rows);
    y_parts.set_size(unique_z.n_rows);
    QVector<QColor> colors(unique_z.n_rows);
    colors.append(QColor(228,26,28));
    colors.append(QColor(55,126,184));
    colors.append(QColor(77,175,74));
    colors.append(QColor(152,78,163));
    colors.append(QColor(255,127,0));
    colors.append(QColor(255,255,51));
    colors.append(QColor(166,86,40));
    colors.append(QColor(247,129,191));
    colors.append(QColor(153,153,153));

    plot_->xAxis->setLabel(x_label);
    plot_->yAxis->setLabel(y_label);
    plot_->xAxis->setRange(x.min(), x.max());
    plot_->yAxis->setRange(y.min(), y.max());
    for (arma::uword i = 0; i < unique_z.n_rows; ++i){
        QPen pen(colors[i]);
        arma::uvec ind = arma::find(z == unique_z(i));
        QVector<double> x_qvec = QVector<double>::fromStdVector(arma::conv_to<stdvec>::from(x(ind)));
        QVector<double> y_qvec = QVector<double>::fromStdVector(arma::conv_to<stdvec>::from(y(ind)));
        plot_->addGraph();
        plot_->graph(i)->setName(QString::number(i));
        plot_->graph(i)->setData(x_qvec, y_qvec);
        plot_->graph(i)->setPen(pen);
        plot_->graph(i)->setScatterStyle(QCPScatterStyle::ssDisc);
        plot_->legend->setVisible(true);
        plot_->graph(i)->addToLegend();
    }
    plot_->replot();
}

PlotViewer::PlotViewer(QWidget *parent, const arma::vec &x, const arma::vec &y, QString x_label, QString y_label) :
    QDialog(parent),
    ui(new Ui::PlotViewer),
    context_menu_("Plot Options", this),
    directory_(QDir::homePath())
{
    ui->setupUi(this);
    plot_ = findChild<QCustomPlot *>("plot");
    plot_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowContextMenu(QPoint)));
    AddContextMenuItems();

    typedef std::vector<double> stdvec;
    QVector<double> x_qvec = QVector<double>::fromStdVector(arma::conv_to<stdvec>::from(x));
    QVector<double> y_qvec = QVector<double>::fromStdVector(arma::conv_to<stdvec>::from(y));
    plot_->addGraph();
    plot_->graph(0)->setData(x_qvec, y_qvec);
    plot_->xAxis->setLabel(x_label);
    plot_->yAxis->setLabel(y_label);
    plot_->xAxis->setRange(x.min(), x.max());
    plot_->yAxis->setRange(y.min(), y.max());
    plot_->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);
    plot_->replot();
}

PlotViewer::~PlotViewer()
{
    delete ui;
}

void PlotViewer::SetLineStyle(QCPGraph::LineStyle ls)
{
    plot_->graph(0)->setLineStyle(ls);
}

void PlotViewer::SetDirectory(QString directory)
{
    directory_ = directory;
}

void PlotViewer::ShowContextMenu(const QPoint &pos)
{
    QPoint global_pos = plot_->mapToGlobal(pos);
    context_menu_.popup(global_pos);
}

void PlotViewer::SaveTriggered()
{
    QString filename =
            QFileDialog::getSaveFileName(this,
                                         tr("Save File"),
                                         directory_,
                                         tr("Scalable Vector Graphics (*.svg);; "
                                            "Enhanced Windows Metafile (*.emf);; "
                                            "Portable Document Format (*.pdf);; "
                                            "Tagged Image File Format (*.tif);; "
                                            "Windows Bitmap (*.bmp);; "
                                            "Portable Network Graphics (*.png);; "
                                            "JPEG (*.jpg)"));
    bool success = Vespucci::SavePlot(plot_, filename);
    if(success)
        QMessageBox::information(this, "Success!", "File " + filename + " written successfully");
    else
        QMessageBox::warning(this, "File Save Failed", "File " + filename + " was not written successfully");
}

void PlotViewer::AddContextMenuItems()
{
    context_menu_.addAction("Save", this, SLOT(SaveTriggered()));
}


