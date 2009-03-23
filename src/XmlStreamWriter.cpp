#include "XmlStreamWriter.h"
#include <QtGui>
#include <iostream>
XmlStreamWriter::XmlStreamWriter(QTreeWidget *tree)
{
	treeWidget = tree;
}

XmlStreamWriter::~XmlStreamWriter(void)
{

}

void XmlStreamWriter::writeIndexEntry(QXmlStreamWriter *xmlWriter, QTreeWidgetItem *item)
{
    xmlWriter->writeStartElement("entry");
    xmlWriter->writeAttribute("parameter", item->text(0));
    QString pageString = item->text(1);
    if (!pageString.isEmpty()) {
        QStringList values = pageString.split(", ");
        foreach (QString value, values)
            xmlWriter->writeTextElement("value", value);
    }
    for (int i = 0; i < item->childCount(); ++i)
        writeIndexEntry(xmlWriter, item->child(i));
    xmlWriter->writeEndElement();
}

bool XmlStreamWriter::writeXml(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        std::cerr << "Error: Cannot write file "
                  << qPrintable(fileName) << ": "
                  << qPrintable(file.errorString()) << std::endl;
        return false;
    }

    QXmlStreamWriter xmlWriter(&file);	
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("ProtocalSettings");
    for (int i = 0; i < treeWidget->topLevelItemCount(); ++i)
        writeIndexEntry(&xmlWriter, treeWidget->topLevelItem(i));
    xmlWriter.writeEndDocument();
    file.close();
    if (file.error()) {
        std::cerr << "Error: Cannot write file "
                  << qPrintable(fileName) << ": "
                  << qPrintable(file.errorString()) << std::endl;
        return false;
    }
    return true;
}
