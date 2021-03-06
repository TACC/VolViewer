// ======================================================================== //
// Copyright 2009-2014 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "TransferFunctionEditor.h"
#include "TransferFunction.h"

static std::string cmap_name(std::string name)
{
	size_t p = name.find_last_of('/');
	if (p != std::string::npos)
			name = name.substr(p+1);
	p = name.find_last_of(".cmap");
	if (p != std::string::npos)
		name = name.substr(0, p-4);
  return name;
}

TransferFunctionEditor::TransferFunctionEditor()
{
	renderer = NULL;

  //! Load color maps.
  loadColorMaps();

  //! Setup UI elments.
  QVBoxLayout * layout = new QVBoxLayout();
  layout->setSizeConstraint(QLayout::SetMinimumSize);
  setLayout(layout);

  //! Form layout.
  QWidget *formWidget = new QWidget();
  QGridLayout *gridLayout = new QGridLayout();
  formWidget->setLayout(gridLayout);
  layout->addWidget(formWidget);

  //! Color map choice.
  for(unsigned int i=0; i<colorMaps.size(); i++)
	{
		std::string name = cmap_name(colorMaps[i].getName());
    colorMapComboBox.addItem(name.c_str());
	}

	gridLayout->addWidget(new QLabel("Colormap"), 0, 0, 1, 1);
	gridLayout->addWidget(&colorMapComboBox, 0, 1, 1, 3);

	gridLayout->addWidget(new QLabel("Min"), 1, 0, 1, 1);
	rangeMin.setValidator(new QDoubleValidator());
	connect(&rangeMin, SIGNAL(returnPressed()), this, SLOT(rangeMinChanged()));
	gridLayout->addWidget(&rangeMin, 1, 1, 1, 1);
	

	gridLayout->addWidget(new QLabel("Max"), 1, 2, 1, 1);
	rangeMax.setValidator(new QDoubleValidator());
	connect(&rangeMax, SIGNAL(returnPressed()), this, SLOT(rangeMaxChanged()));
	gridLayout->addWidget(&rangeMax, 1, 3, 1, 1);

  //! Widget containing opacity transfer function widget and scaling slider.
  QWidget * transferFunctionAlphaGroup = new QWidget();
  QHBoxLayout * hboxLayout = new QHBoxLayout();
  transferFunctionAlphaGroup->setLayout(hboxLayout);

  //! Opacity transfer function widget.
  hboxLayout->addWidget(&transferFunctionAlphaWidget);

  //! Opacity scaling slider, defaults to median value in range.
  transferFunctionAlphaScalingSlider.setValue(int(transferFunction.GetScale() * (transferFunctionAlphaScalingSlider.minimum() + transferFunctionAlphaScalingSlider.maximum())));
  transferFunctionAlphaScalingSlider.setOrientation(Qt::Vertical);
  hboxLayout->addWidget(&transferFunctionAlphaScalingSlider);

  gridLayout->addWidget(transferFunctionAlphaGroup, 2, 0, 1, 4);
  connect(&colorMapComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setColorMapIndex(int)));
  connect(&transferFunctionAlphaWidget, SIGNAL(transferFunctionChanged()), this, SLOT(alphaWidgetChanged()));
  connect(&transferFunctionAlphaScalingSlider, SIGNAL(valueChanged(int)), this, SLOT(alphaWidgetChanged()));

	setColorMapIndex(0);
}

void TransferFunctionEditor::alphaWidgetChanged()
{
	std::vector< osp::vec2f > alphas;
	QVector< QPointF > widgetAlphas = transferFunctionAlphaWidget.getPoints();

	for (int i = 0; i < widgetAlphas.size(); i++)
		alphas.push_back(osp::vec2f(widgetAlphas[i].x(), widgetAlphas[i].y()));

	getTransferFunction().SetAlphas(alphas);

	float scale = float(transferFunctionAlphaScalingSlider.value() - transferFunctionAlphaScalingSlider.minimum()) / float(transferFunctionAlphaScalingSlider.maximum() - transferFunctionAlphaScalingSlider.minimum());


	getTransferFunction().SetScale(scale);

	modifiedTransferFunction();
}

void TransferFunctionEditor::modifiedTransferFunction() {
	if (renderer) transferFunction.commit(renderer);
  emit transferFunctionChanged();
}

void TransferFunctionEditor::loadColorMap()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Load colormap"), ".", "colormap files (*.cmap)");

  if(filename.isEmpty())
    return;

	loadColorMapFile(filename.toStdString());
}

void TransferFunctionEditor::loadColorMapFile(string filename)
{
	VColorMap cmap(filename);
	addColorMap(cmap);
}

void TransferFunctionEditor::addColorMap(VColorMap cmap)
{
	colorMaps.push_back(cmap);
  colorMapComboBox.addItem(cmap_name(colorMaps[colorMaps.size()-1].getName()).c_str());
  colorMapComboBox.setCurrentIndex(colorMaps.size()-1);
}

void TransferFunctionEditor::_setRange(float min, float max)
{
	rangeMin.setText(QString::number(min));
	rangeMax.setText(QString::number(max));
}

void TransferFunctionEditor::setRange(float min, float max)
{
	transferFunction.SetMin(min);
	transferFunction.SetMax(max);
	_setRange(min, max);
}

void TransferFunctionEditor::loadState(Value& in)
{
	transferFunction.loadState(in);
	
	setRange(transferFunction.GetMin(), transferFunction.GetMax());

  transferFunctionAlphaScalingSlider.setValue(int(transferFunction.GetScale() * (transferFunctionAlphaScalingSlider.minimum() + transferFunctionAlphaScalingSlider.maximum())));

	QVector<QPointF> points;
	for (int i = 0; i < transferFunction.GetAlphas().size(); i++)
		points.push_back(QPointF(transferFunction.GetAlphas()[i].x, transferFunction.GetAlphas()[i].y));

  transferFunctionAlphaWidget.setPoints(points);

	if (in.HasMember("Colormap"))
	{
		ColorMap cmap;
		cmap.loadState(in["Colormap"]);

		int colorMapIndex;
		for(colorMapIndex = 0; colorMapIndex < colorMaps.size(); colorMapIndex++)
			if (colorMaps[colorMapIndex].getName() == cmap.getName())
				break;

		if (colorMapIndex == colorMaps.size())
			addColorMap(VColorMap(cmap));

		colorMapComboBox.setCurrentIndex(colorMapIndex);
	}

}

void TransferFunctionEditor::rangeMinChanged()
{
	float a = atof(rangeMin.text().toStdString().c_str());
	getTransferFunction().SetMin(a);
	modifiedTransferFunction();
}

void TransferFunctionEditor::rangeMaxChanged()
{
	float a = atof(rangeMax.text().toStdString().c_str());
	getTransferFunction().SetMax(a);
	modifiedTransferFunction();
}

void TransferFunctionEditor::saveState(Document& doc, Value &out)
{
	transferFunction.saveState(doc, out);
	colorMaps[colorMapComboBox.currentIndex()].getColorMap().saveState(doc, out["TransferFunction"]);
}

void TransferFunctionEditor::setColorMapIndex(int index)
{
	colorMaps[index].commit(transferFunction);
  transferFunctionAlphaWidget.setBackgroundImage(colorMaps[index].getImage());
	modifiedTransferFunction();
}

void TransferFunctionEditor::loadColorMaps() {

	std::vector< ColorMap > cmaps = ColorMap::load_colormap_directory();

  for (std::vector< ColorMap >::iterator it = cmaps.begin(); it != cmaps.end(); ++it)
	{
		VColorMap vc(*it);
		colorMaps.push_back(vc);
	}
}
