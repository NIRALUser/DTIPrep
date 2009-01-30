#pragma once

#include <QXmlStreamWriter>

class QTreeWidget;
class QTreeWidgetItem;
class XmlStreamWriter
{
public:
	XmlStreamWriter(QTreeWidget *tree);
	~XmlStreamWriter(void);
    
	 bool writeXml(const QString &fileName);

 private:
     void writeIndexEntry(QXmlStreamWriter *xmlWriter, QTreeWidgetItem *item);

     QTreeWidget *treeWidget;
};


 