#include "XmlStreamReader.h"
#include <QtGui>

#include <iostream>



XmlStreamReader::XmlStreamReader(QTreeWidget *tree)
{
	treeWidget = tree;
	protocal=NULL;
}

XmlStreamReader::~XmlStreamReader(void)
{
}

bool XmlStreamReader::readFile(const QString &fileName, int mode)
{
	if(protocal)	protocal->clear();

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        std::cerr << "Error: Cannot read file " << qPrintable(fileName)
                  << ": " << qPrintable(file.errorString())
                  << std::endl;
        return false;
    }
    reader.setDevice(&file);

	reader.readNext();
	while (!reader.atEnd()) {
		if (reader.isStartElement()) {
			if (reader.name() == "ProtocalSettings") {
				readProtocalSettingsElement( mode );
			} else {
				reader.raiseError(QObject::tr("Not a ProtocalSettings file"));
			}
		} else {
			reader.readNext();
		}
	}


    file.close();
    if (reader.hasError()) {
        std::cerr << "Error: Failed to parse file "
                  << qPrintable(fileName) << ": "
                  << qPrintable(reader.errorString()) << std::endl;
        return false;
    } else if (file.error() != QFile::NoError) {
        std::cerr << "Error: Cannot read file " << qPrintable(fileName)
                  << ": " << qPrintable(file.errorString())
                  << std::endl;
        return false;
    }
    return true;
}

void XmlStreamReader::readProtocalSettingsElement(int mode)
{
    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "entry") {
				if( mode== XmlStreamReader::TreeWise)
					readEntryElement(treeWidget->invisibleRootItem());
				else if(mode== XmlStreamReader::ProtocalWise){
					readEntryElement();					
				}
				else{
					std::cout<<"invalid setting reading mode"<<std::endl;
					return;
				}
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }

	if(mode== XmlStreamReader::ProtocalWise) 
	{
		Initialize();
		parseparametersToProtocal();
	}
}

void XmlStreamReader::parseparametersToProtocal()
{
	
	int temp;
	QStringList values;
	QStringList subvalues;
	for(unsigned int i=0;i<paremeters.size();i++)
	{
		std::cout<<paremeters[i].parameter.toStdString()<<"    "<<paremeters[i].value.toStdString()<<std::endl;
		if(paremeters[i].parameter.left(14)==QObject::tr("DWMRI_gradient"))
		{
			std::vector<double> vect;
			values = paremeters[i].value.split(" ");
			foreach (QString value, values)
			{
				vect.push_back( value.toDouble());
			}
			protocal->GetDiffusionProtocal().gradients.push_back(vect);
		}
		else
		{
			switch(s_mapStringValues[paremeters[i].parameter.toStdString()])
			{
			case ev_ReportFileName:
				protocal->GetReportFileName() =  paremeters[i].value.toStdString();
				break;

			case ev_Image:
				if(paremeters[i].value.toStdString()=="Yes" || paremeters[i].value.toStdString()=="YES" ||paremeters[i].value.toStdString()=="yes")
				{
					protocal->GetImageProtocal().bCheck = true;
					
					//if(protocal->GetImageProtocal().bCheck)
					//	std::cout<<"protocal->GetImageProtocal().bCheck = true;"<<std::endl;
				}
				else{
					protocal->GetImageProtocal().bCheck = false;
					//std::cout<<"protocal->GetImageProtocal().bCheck = true;"<<std::endl;
				}
				break;
		//	case ev_type:
		//		if(paremeters[i].value.toStdString()=="short")
		//			protocal->GetImageProtocal().type = Protocal::TYPE_SHORT;
		//		else if(paremeters[i].value.toStdString()=="unsigned short")
		//			protocal->GetImageProtocal().type = Protocal::TYPE_USHORT;
		//		else
		//			protocal->GetImageProtocal().type = Protocal::TYPE_UNKNOWN;
		//		break;
			case ev_space:
				if(paremeters[i].value.toStdString()=="left-anterior-inferior")
					protocal->GetImageProtocal().space = Protocal::SPACE_LAI;
				else if(paremeters[i].value.toStdString()=="left-anterior-superior")
					protocal->GetImageProtocal().space = Protocal::SPACE_LAS;
				else if(paremeters[i].value.toStdString()=="left-posterior-inferior")
					protocal->GetImageProtocal().space = Protocal::SPACE_LPI;
				else if(paremeters[i].value.toStdString()=="left-posterior-superior")
					protocal->GetImageProtocal().space = Protocal::SPACE_LPS;
				else if(paremeters[i].value.toStdString()=="right-anterior-inferior")
					protocal->GetImageProtocal().space = Protocal::SPACE_RAI;
				else if(paremeters[i].value.toStdString()=="right-anterior-superior")
					protocal->GetImageProtocal().space = Protocal::SPACE_RAS;
				else if(paremeters[i].value.toStdString()=="right-posterior-inferior")
					protocal->GetImageProtocal().space = Protocal::SPACE_RPI;
				else if(paremeters[i].value.toStdString()=="right-posterior-superior")
					protocal->GetImageProtocal().space = Protocal::SPACE_RPS;
				else
					protocal->GetImageProtocal().space = Protocal::SPACE_UNKNOWN;
				break;
			case ev_directions:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					subvalues = value.split(" ");
					foreach (QString subvalue, subvalues)
					{
						protocal->GetImageProtocal().spacedirection[temp/3][temp%3]= subvalue.toDouble();
						temp++;
					}
				}				
				break;
			case ev_dimension:
				protocal->GetImageProtocal().dimension = paremeters[i].value.toInt();
				break;
			case ev_sizes:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					protocal->GetImageProtocal().size[temp]= value.toInt();
					temp++;
				}				
				break;
			case ev_spacing:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					protocal->GetImageProtocal().spacing[temp]= value.toDouble();
					temp++;
				}				
				break;
			case ev_origin:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					protocal->GetImageProtocal().origin[temp]= value.toDouble();
					temp++;
				}				
				break;
			case ev_Diffusion:
				if(paremeters[i].value.toStdString()=="Yes" || paremeters[i].value.toStdString()=="YES" ||paremeters[i].value.toStdString()=="yes")
					protocal->GetDiffusionProtocal().bCheck = true;
				else
					protocal->GetDiffusionProtocal().bCheck = false;
				break;
			case ev_measurementFrame:
				values = paremeters[i].value.split(", ");
				temp=0;
				foreach (QString value, values)
				{
					subvalues = value.split(" ");
					foreach (QString subvalue, subvalues)
					{
						protocal->GetDiffusionProtocal().measurementFrame[temp/3][temp%3]= subvalue.toDouble();
						temp++;
					}
				}				
				break;
			case ev_bvalue:
				protocal->GetDiffusionProtocal().b=  paremeters[i].value.toInt();
				break;
			//case ev_DWMRI_gradient:
			//	break;
			case ev_QCCheck:
				if(paremeters[i].value.toStdString()=="Yes" || paremeters[i].value.toStdString()=="YES" ||paremeters[i].value.toStdString()=="yes")
					protocal->GetIntensityMotionCheckProtocal().bCheck = true;
				else
					protocal->GetIntensityMotionCheckProtocal().bCheck = false;
				break;
			case ev_QCOutputFileName:
				protocal->GetIntensityMotionCheckProtocal().OutputFileName =  paremeters[i].value.toStdString();
				break;
			case ev_SliceWiseCheck:
				if(paremeters[i].value.toStdString()=="Yes" || paremeters[i].value.toStdString()=="YES" ||paremeters[i].value.toStdString()=="yes")
					protocal->GetIntensityMotionCheckProtocal().bSliceCheck = true;
				else
					protocal->GetIntensityMotionCheckProtocal().bSliceCheck = false;
				break;
			case ev_sliceCorrelationThreshold:
				protocal->GetIntensityMotionCheckProtocal().sliceCorrelationThreshold = paremeters[i].value.toDouble(); 
				break;
			case ev_sliceCorrelationDeviationThreshold:
				protocal->GetIntensityMotionCheckProtocal().sliceCorrelationDeviationThreshold = paremeters[i].value.toDouble(); 
				break;
			case ev_badSlicePercentageTolerance:
				protocal->GetIntensityMotionCheckProtocal().badSlicePercentageTolerance = paremeters[i].value.toDouble(); 
				break;
			case ev_InterlaceWiseCheck:
				if(paremeters[i].value.toStdString()=="Yes" || paremeters[i].value.toStdString()=="YES" ||paremeters[i].value.toStdString()=="yes")
					protocal->GetIntensityMotionCheckProtocal().bInterlaceCheck = true;
				else
					protocal->GetIntensityMotionCheckProtocal().bInterlaceCheck = false;
				break;
			case ev_interlaceTranslationThreshold:
				protocal->GetIntensityMotionCheckProtocal().interlaceTranslationThreshold = paremeters[i].value.toDouble(); 
				break;
			case ev_interlaceRotationThreshold:
				protocal->GetIntensityMotionCheckProtocal().interlaceRotationThreshold = paremeters[i].value.toDouble(); 
				break;
			case ev_interlaceCorrelationThresholdBaseline:
				protocal->GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdBaseline = paremeters[i].value.toDouble(); 
				break;
			case ev_interlaceCorrelationThresholdGradient:
				protocal->GetIntensityMotionCheckProtocal().interlaceCorrelationThresholdGradient = paremeters[i].value.toDouble(); 
				break;
			case ev_GradientWiseCheck:
				if(paremeters[i].value.toStdString()=="Yes" || paremeters[i].value.toStdString()=="YES" ||paremeters[i].value.toStdString()=="yes")
					protocal->GetIntensityMotionCheckProtocal().bGradientCheck = true;
				else
					protocal->GetIntensityMotionCheckProtocal().bGradientCheck = false;
				break;
			case ev_gradientTranslationThrehshold:
				protocal->GetIntensityMotionCheckProtocal().gradientTranslationThreshold = paremeters[i].value.toDouble(); 
				break;
			case ev_gradientRotationThreshold:
				protocal->GetIntensityMotionCheckProtocal().gradientRotationThreshold = paremeters[i].value.toDouble(); 
				break;
///
			case ev_EddyMotionCorrection:
				if(paremeters[i].value.toStdString()=="Yes")
					protocal->GetEddyMotionCorrectionProtocal().bCorrect = true;
				else
					protocal->GetEddyMotionCorrectionProtocal().bCorrect = false;
				break;

			case ev_EddyMotionCommand:
				protocal->GetEddyMotionCorrectionProtocal().EddyMotionCommand =  paremeters[i].value.toStdString();
				break;

			case ev_EddyMotionOutputFileName:
				protocal->GetEddyMotionCorrectionProtocal().OutputFileName =  paremeters[i].value.toStdString();
				break;
			case ev_EddyMotionInputFileName:
				protocal->GetEddyMotionCorrectionProtocal().InputFileName =  paremeters[i].value.toStdString();
				break;
///
			case ev_DTIComputing:
				if(paremeters[i].value.toStdString()=="Yes")
					protocal->GetDTIProtocal().bCompute = true;
				else
					protocal->GetDTIProtocal().bCompute = false;
				break;

			case ev_dtiestimCommand:
				protocal->GetDTIProtocal().dtiestimCommand =  paremeters[i].value.toStdString();
				break;
			case ev_dtiprocessCommand:
				protocal->GetDTIProtocal().dtiprocessCommand =  paremeters[i].value.toStdString();
				break;

			case ev_DTIMethod:
				if(paremeters[i].value.toStdString()=="wls")
					protocal->GetDTIProtocal().method = Protocal::METHOD_WLS;
				else if(paremeters[i].value.toStdString()=="lls")
					protocal->GetDTIProtocal().method = Protocal::METHOD_LLS;
				else if(paremeters[i].value.toStdString()=="ml")
					protocal->GetDTIProtocal().method = Protocal::METHOD_ML;
				else
					protocal->GetDTIProtocal().method = Protocal::METHOD_UNKNOWN;
				break;
			case ev_baselineThreshold:
				protocal->GetDTIProtocal().baselineThreshold=  paremeters[i].value.toInt();
				break;
			case ev_maskFIle:
				protocal->GetDTIProtocal().mask =  paremeters[i].value.toStdString();
				break;
			case ev_tensor:
				protocal->GetDTIProtocal().tensor = paremeters[i].value.toStdString();
				break;
			case ev_fa:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().fa = values[1].toStdString();
				if(values[0].toStdString()=="Yes")
				{
					protocal->GetDTIProtocal().bfa = true;
				}
				else
				{
					protocal->GetDTIProtocal().bfa = false;
				}
				break;
			case ev_md:
				values = paremeters[i].value.split(", ");
					protocal->GetDTIProtocal().md = values[1].toStdString();
				if(values[0].toStdString()=="Yes")
					protocal->GetDTIProtocal().bmd = true;
				else
					protocal->GetDTIProtocal().bmd = false;
				break;
			case ev_colorfa:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().coloredfa = values[1].toStdString();
				if(values[0].toStdString()=="Yes")
					protocal->GetDTIProtocal().bcoloredfa= true;
				else
					protocal->GetDTIProtocal().bcoloredfa= false;
				break;


			case ev_idwi:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().idwi  = values[1].toStdString();
				if(values[0].toStdString()=="Yes")
					protocal->GetDTIProtocal().bidwi = true;
				else
					protocal->GetDTIProtocal().bidwi = false;
				break;

			case ev_frobeniusnorm:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().frobeniusnorm = values[1].toStdString();
				if(values[0].toStdString()=="Yes")
					protocal->GetDTIProtocal().bfrobeniusnorm = true;
				else
					protocal->GetDTIProtocal().bfrobeniusnorm = false;
				break;

			case ev_baseline:
				values = paremeters[i].value.split(", ");
				protocal->GetDTIProtocal().baseline  = values[1].toStdString();
				if(values[0].toStdString()=="Yes")
					protocal->GetDTIProtocal().bbaseline = true;
				else
					protocal->GetDTIProtocal().bbaseline = false;
				break;


			case ev_Unknow:
			default:
				break;
			}


		}

	
	}

}



void XmlStreamReader::readEntryElement(QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, reader.attributes().value("parameter").toString());

    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "entry") {
                readEntryElement(item);
            } else if (reader.name() == "value") {
                readValueElement(item);
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }
}

 void XmlStreamReader::readEntryElement()
{
	ITEM item;

	item.parameter=reader.attributes().value("parameter").toString();
	paremeters.push_back( item );

    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            if (reader.name() == "entry") {
                readEntryElement();
            } else if (reader.name() == "value") {
                readValueElement();
            } else {
                skipUnknownElement();
            }
        } else {
            reader.readNext();
        }
    }
}

 void XmlStreamReader::readValueElement()
{
    QString page = reader.readElementText();
    if (reader.isEndElement())
        reader.readNext();

	QString allPages =paremeters[paremeters.size()-1].value;
    if (!allPages.isEmpty())
        allPages += ", ";
    allPages += page;

	paremeters[paremeters.size()-1].value = allPages;
   
}

void XmlStreamReader::readValueElement(QTreeWidgetItem *parent)
{
    QString page = reader.readElementText();
    if (reader.isEndElement())
        reader.readNext();

    QString allPages = parent->text(1);
    if (!allPages.isEmpty())
        allPages += ", ";
    allPages += page;

    parent->setText(1, allPages);
}

void XmlStreamReader::skipUnknownElement()
{
    reader.readNext();
    while (!reader.atEnd()) {
        if (reader.isEndElement()) {
            reader.readNext();
            break;
        }

        if (reader.isStartElement()) {
            skipUnknownElement();
        } else {
            reader.readNext();
        }
    }
}















