#pragma once

#include <QXmlStreamWriter>
#include <QMap>
#include "Protocol.h"

class QTreeWidget;
class QTreeWidgetItem;

class XmlStreamWriter
  {
public:
  XmlStreamWriter(QTreeWidget *tree);
  ~XmlStreamWriter(void);

  bool writeXml(const QString & fileName);

  void setProtocol( Protocol  *p )
  {
    protocol = p;
  };

private:
  void writeIndexEntry(QXmlStreamWriter *xmlWriter, QTreeWidgetItem *item);

  QTreeWidget *treeWidget;
  Protocol *protocol;

  

 };
