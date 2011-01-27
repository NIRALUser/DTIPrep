#include "XmlStreamWriter.h"
#include <QtGui>
#include <iostream>
#include <string>
#include <math.h>


XmlStreamWriter::XmlStreamWriter(QTreeWidget *tree)
  {
  treeWidget = tree;
  protocol = NULL;
  }

XmlStreamWriter::~XmlStreamWriter(void)
     {}

void XmlStreamWriter::writeIndexEntry(QXmlStreamWriter *xmlWriter,
  QTreeWidgetItem *item)
{
  if ( !protocol )
  {
    std::cout << "protocol not set!" << std::endl;
    return;
  }

  xmlWriter->writeStartElement("entry");
  xmlWriter->writeAttribute( "parameter", item->text(0) );
  
  QString pageString_processing =item->text(2);
  if ( !pageString_processing.isEmpty())
    {
    xmlWriter->writeTextElement("processing",pageString_processing);
    }

  bool m_setThreshold=false;

  QString pageString = item->text(0);
  if  (pageString== "InterlaceAngleX" || pageString=="InterlaceAngleY" || pageString=="InterlaceAngleZ")
       {
         if ( item->text(1).toFloat() < protocol->GetInterlaceCheckProtocol().rotationThreshold) // rotation threshold in protocol
             xmlWriter->writeTextElement("green",item->text(1));
         else 
             xmlWriter->writeTextElement("red",item->text(1));
         m_setThreshold=true;
       }
  if (pageString=="InterlaceTranslationX" || pageString=="InterlaceTranslationY" || pageString=="InterlaceTranslationZ")
       {
         if ( item->text(1).toFloat() < protocol->GetInterlaceCheckProtocol().translationThreshold) // translation threshold in protocol
             xmlWriter->writeTextElement("green",item->text(1));
         else 
             xmlWriter->writeTextElement("red",item->text(1));
         m_setThreshold=true;

       }
  if (pageString=="InterlaceCorrelation_Baseline")
      {
        if (item->text(1).toFloat() > protocol->GetInterlaceCheckProtocol().correlationThresholdBaseline)   //correlation threshold for baseline in protocol
        {
            xmlWriter->writeTextElement("green",item->text(1));
            std::cout<<"InterlaceCorrelation_Baseline"<<protocol->GetInterlaceCheckProtocol().correlationThresholdBaseline<<std::endl;
        }
        else 
            xmlWriter->writeTextElement("red",item->text(1));
        m_setThreshold=true;

      }

  if (pageString=="InterlaceCorrelation")
      {
        if (item->text(1).toFloat() > protocol->GetInterlaceCheckProtocol().correlationThresholdGradient)  //correlation threshold for gradients in protocol
            xmlWriter->writeTextElement("green",item->text(1));
        else 
            xmlWriter->writeTextElement("red",item->text(1));
        m_setThreshold=true;

      }
    
  if  (pageString=="GradientAngleX" || pageString=="GradientAngleY" || pageString=="GradientAngleZ")
       {
         if ( item->text(1).toFloat() < protocol->GetGradientCheckProtocol().rotationThreshold) // rotation threshold in protocol
             xmlWriter->writeTextElement("green",item->text(1));
         else 
             xmlWriter->writeTextElement("red",item->text(1));
         m_setThreshold=true;


       }
  if  (pageString=="GradientTranslationX" || pageString=="GradientTranslationY" || pageString=="GradientTranslationZ")
       {
         if ( item->text(1).toFloat() < protocol->GetGradientCheckProtocol().translationThreshold) // translation threshold in protocol
             xmlWriter->writeTextElement("green",item->text(1));
         else 
             xmlWriter->writeTextElement("red",item->text(1));
         m_setThreshold=true;

       }
  
  if (m_setThreshold==false) {

     QString pageStringValue = item->text(1);
     if ( !pageStringValue.isEmpty() )
        {
        QStringList values = pageStringValue.split(", ");
        foreach (QString value, values)
        xmlWriter->writeTextElement("value", value);
        }
  }
  for ( int i = 0; i < item->childCount(); ++i )
        {
        writeIndexEntry( xmlWriter, item->child(i) );
        }
  xmlWriter->writeEndElement();
  
}

bool XmlStreamWriter::writeXml(const QString & fileName)
{
  QFile file(fileName);

  if ( !file.open(QFile::WriteOnly | QFile::Text) )
    {
    std::cerr << "Error: Cannot write file "
              << qPrintable(fileName) << ": "
              << qPrintable( file.errorString() ) << std::endl;
    return false;
    }

  QXmlStreamWriter xmlWriter(&file);
  xmlWriter.setAutoFormatting(true);
  xmlWriter.writeStartDocument();
  xmlWriter.writeStartElement("QCResultSettings");
  for ( int i = 0; i < treeWidget->topLevelItemCount(); ++i )
    {
    writeIndexEntry( &xmlWriter, treeWidget->topLevelItem(i) );
    }
  xmlWriter.writeEndDocument();
  file.close();
  if ( file.error() )
    {
    std::cerr << "Error: Cannot write file "
              << qPrintable(fileName) << ": "
              << qPrintable( file.errorString() ) << std::endl;
    return false;
    }
  return true;
}
