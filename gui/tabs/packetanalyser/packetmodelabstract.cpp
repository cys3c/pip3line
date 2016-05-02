/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "packetmodelabstract.h"
#include "packet.h"
#include <QDebug>
#include <QChar>
#include <QIcon>
#include <transformmgmt.h>
#include <threadedprocessor.h>
#include "shared/guiconst.h"

const int PacketModelAbstract::COLUMN_DIRECTION = 0;
const int PacketModelAbstract::COLUMN_TIMESPTAMP = 1;
const int PacketModelAbstract::COLUMN_PAYLOAD = 2;
const int PacketModelAbstract::COLUMN_COMMENT = 3;
const int PacketModelAbstract::COLUMN_INVALID = -1;
const qint64 PacketModelAbstract::INVALID_POS = -1;
const QString PacketModelAbstract::COLUMN_DIRECTION_STR = "D";
const QString PacketModelAbstract::COLUMN_TIMESPTAMP_STR = "Timestamp";
const QString PacketModelAbstract::COLUMN_PAYLOAD_STR = "Payload";
const QString PacketModelAbstract::COLUMN_COMMENT_STR = "Comment";
const QString PacketModelAbstract::DEFAULT_DATETIME_FORMAT = "dd/MM/yyyy hh:mm:ss.zzz";

PacketModelAbstract::PacketModelAbstract(TransformMgmt *transformFactory, QObject *parent) :
    QAbstractTableModel(parent),
    transformFactory(transformFactory)
{
    autoMergeConsecutivePackets = false;
    columnNames << COLUMN_DIRECTION_STR << COLUMN_TIMESPTAMP_STR << COLUMN_PAYLOAD_STR << COLUMN_COMMENT_STR;
    dateTimeFormat = DEFAULT_DATETIME_FORMAT;

    Usercolumn uc;
    uc.format = Pip3lineConst::HEXAFORMAT;
    userColumnsDef.insert(COLUMN_PAYLOAD_STR, uc);

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onRowsInserted(QModelIndex,int,int)));
}

PacketModelAbstract::~PacketModelAbstract()
{
    clearUserColumns();
}

void PacketModelAbstract::resetColumnNames()
{
    if (columnNames.size() > COLUMN_COMMENT + 1) { // difference between index and size
        columnNames.clear();
        columnNames << COLUMN_DIRECTION_STR << COLUMN_TIMESPTAMP_STR << COLUMN_PAYLOAD_STR << COLUMN_COMMENT_STR;
    }
}
bool PacketModelAbstract::isAutoMergeConsecutivePackets() const
{
    return autoMergeConsecutivePackets;
}

void PacketModelAbstract::setAutoMergeConsecutivePackets(bool value)
{
    autoMergeConsecutivePackets = value;
}

QStringList PacketModelAbstract::getColumnNames() const
{
    return columnNames;
}

void PacketModelAbstract::setColumnNames(const QStringList &value)
{
    columnNames = value;
}

int PacketModelAbstract::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return columnNames.size();
}

QVariant PacketModelAbstract::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        return section < columnNames.size() ? QVariant(columnNames.at(section)) : QVariant();
    } else {
        return  QVariant(section + 1);
    }
}

QVariant PacketModelAbstract::payloadData(const Packet * packet, int column, int role) const
{
    if (packet == nullptr)
        return QVariant();

    // Only deal with DisplayRole, DecorationRole, BackgroundRole and ForegroundRole

    if (role == Qt::DisplayRole) {
        if ( column == COLUMN_TIMESPTAMP) {
            QString final = packet->getTimestamp().toString(dateTimeFormat);
            if (dateTimeFormat == DEFAULT_DATETIME_FORMAT && packet->getMicrosec() != 0) {
                final = QString("%1%2").arg(final).arg(packet->getMicrosec(),3,10,QChar('0'));
            }
            return QVariant(final);
        }
        else if (column == COLUMN_PAYLOAD) {
            OutputFormat of = userColumnsDef.value(COLUMN_PAYLOAD_STR,Usercolumn()).format;
            return QVariant(of == Pip3lineConst::TEXTFORMAT ? QString::fromUtf8(packet->getData()): QString::fromUtf8(packet->getData().toHex()));
        }
        else if (column == COLUMN_COMMENT)
            return QVariant(packet->getComment());
        else if (column > COLUMN_COMMENT && column < columnNames.size()) {
            QHash<QString, QString> fields = packet->getAdditionalFields();
            if (fields.contains(columnNames.at(column))) {
                return QVariant(fields.value(columnNames.at(column)));
            }
        }
    } else if (role == Qt::DecorationRole) {
        if (column == COLUMN_DIRECTION) {
            switch (packet->getDirection()) {
                case Packet::LEFTRIGHT:
                    if (packet->isInjected())
                        return QVariant(QIcon(":/Images/icons/arrow-right-3-1.png"));
                    else
                        return QVariant(QIcon(":/Images/icons/arrow-right-3-mod.png"));
                case Packet::RIGHTLEFT:
                    if (packet->isInjected())
                        return QVariant(QIcon(":/Images/icons/arrow-left-3-1.png"));
                    else
                        return QVariant(QIcon(":/Images/icons/arrow-left-3.png"));
                case Packet::NODIRECTION:
                    return QVariant();
            }
        }
    } else if (role == Qt::BackgroundRole) {
        QColor color = packet->getBackground();
        return color.isValid() ? QVariant(color) : QVariant();
    } else if (role == Qt::ForegroundRole) {
        QColor color = packet->getForeground();
        return color.isValid() ? QVariant(color) : QVariant();
    } else if (role == Qt::ToolTipRole) {
        return packet->getSourceString();
    } else if (role == Qt::FontRole) {
        return GuiStyles::GLOBAL_REGULAR_FONT;
    }

    return QVariant();
}

Packet *PacketModelAbstract::getPacket(const QModelIndex &index)
{
    if (index.isValid()) {
        return getPacket((qint64)index.row());
    }

    return nullptr;
}

qint64 PacketModelAbstract::indexToPacketIndex(const QModelIndex &index)
{
    if (index.isValid()) {
        // by default nothing to do
        return (qint64)index.row();
    }

    return INVALID_POS;
}

QModelIndex PacketModelAbstract::createIndex(int row, int column) const
{
    return QAbstractTableModel::createIndex(row, column);
}

void PacketModelAbstract::addUserColumn(const QString &name, TransformAbstract *transform, OutputFormat outFormat)
{
    if (userColumnsDef.contains(name)) {
        emit log(tr("A column with this name \"%1\" already exists, ignoring").arg(name), "PacketModel", Pip3lineConst::LERROR);
        delete transform;
        return;
    }
    Usercolumn col;
    col.transform = transform;
    col.format = outFormat;

    userColumnsDef.insert(name,col);
    int newIndex = columnNames.size();
    beginInsertColumns(QModelIndex(),newIndex,newIndex);
    columnNames << name;
    endInsertColumns();
    if (transform != nullptr) {
        connect(transform, SIGNAL(confUpdated()), SLOT(transformUpdated()));
        internalAddUserColumn(name,transform);
    }
    emit columnsUpdated();
}

void PacketModelAbstract::removeUserColumn(const QString &name)
{
    removeUserColumn(columnNames.indexOf(name));
}

void PacketModelAbstract::removeUserColumn(const int &index)
{
    if (index > COLUMN_COMMENT && index < columnNames.size()) {
        beginRemoveColumns(QModelIndex(), index, index);
        QString name = columnNames.at(index);
        if (userColumnsDef.contains(name)) {
            Usercolumn col = userColumnsDef.take(name);
            delete col.transform;
        } else {
            qWarning() << tr("[PacketModelAbstract::removeUserColumn] Column name not found in the definitions %1 T_T").arg(name);
        }
        columnNames.removeAt(index);
        endRemoveColumns();
        clearAdditionalField(name);
        emit columnsUpdated();
    } else {
        qCritical() << tr("[PacketModelAbstract::removeUserColumn] No column at index %1 T_T").arg(index);
    }
}

void PacketModelAbstract::clearUserColumns()
{
    foreach (Usercolumn col, userColumnsDef)
        delete col.transform;

    userColumnsDef.clear();
    resetColumnNames();
    emit columnsUpdated();
}

bool PacketModelAbstract::isUserColumn(const QString &name) const
{
    return isUserColumn(columnNames.indexOf(name));
}

bool PacketModelAbstract::isUserColumn(int column) const
{
    return column > COLUMN_COMMENT && column < columnNames.size();
}

bool PacketModelAbstract::isDefaultColumn(const QString &name) const
{
    return isDefaultColumn(columnNames.indexOf(name));
}

bool PacketModelAbstract::isDefaultColumn(int column) const
{
    return column >= 0 && column <= COLUMN_COMMENT;
}

int PacketModelAbstract::getColumnIndex(const QString &name)
{
    int index = columnNames.indexOf(name);
    if (index == -1) {
        qCritical() << tr("[PacketModelAbstract::getColumnIndex] No column named %1 T_T").arg(name);
        return COLUMN_INVALID;
    }

    return index;
}

void PacketModelAbstract::refreshAllColumn()
{
    for (int i = COLUMN_COMMENT + 1 ; i < columnNames.size(); i++) {
        if (userColumnsDef.contains(columnNames.at(i))) {
            TransformAbstract * transform = userColumnsDef.value(columnNames.at(i)).transform;
            launchUpdate(transform,0,i);
        }
    }
}

void PacketModelAbstract::onRowsInserted(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid()) // not supposed to hapen for a table
        return;

    for (int i = COLUMN_COMMENT + 1 ; i < columnNames.size(); i++) {
        if (userColumnsDef.contains(columnNames.at(i))) {
            TransformAbstract * transform = userColumnsDef.value(columnNames.at(i)).transform;
            launchUpdate(transform, start,i, end - start + 1);
        }
    }
}

QWidget *PacketModelAbstract::getGuiForUserColumn(const QString &name, QWidget * parent)
{
    if (userColumnsDef.contains(name)) {
        TransformAbstract * ta = userColumnsDef.value(name).transform;
        return ( ta != nullptr ? ta->getGui(parent) : nullptr);
    } else {
        qCritical() << tr("[PacketModelAbstract::getGuiForUserColumn] Cannot find the column name (%1) T_T").arg(name);
    }
    return nullptr;
}

QWidget *PacketModelAbstract::getGuiForUserColumn(int index, QWidget *parent)
{

    if (index > COLUMN_COMMENT && index < columnNames.size()) {
        return getGuiForUserColumn(columnNames.at(index),parent);
    } else {
        qCritical() << tr("[PacketModelAbstract::getGuiForUserColumn] Cannot find the column at (%1) T_T").arg(index);
    }

    return nullptr;
}

TransformAbstract *PacketModelAbstract::getTransform(const QString &columnName)
{
    if (userColumnsDef.contains(columnName)) {
        return userColumnsDef.value(columnName).transform;
    } else {
        qCritical() << tr("[PacketModelAbstract::getTransform] Cannot find the column name (%1) T_T").arg(columnName);
    }
    return nullptr;
}

TransformAbstract *PacketModelAbstract::getTransform(int index)
{
    if (index > COLUMN_COMMENT && index < columnNames.size()) {
        return getTransform(columnNames.at(index));
    } else {
        qCritical() << tr("[PacketModelAbstract::getTransform] Cannot find the column at (%1) T_T").arg(index);
    }

    return nullptr;
}

void PacketModelAbstract::setColumnFormat(int index, OutputFormat format)
{
    if ((index > COLUMN_COMMENT && index < columnNames.size()) || index == COLUMN_PAYLOAD) {
        setColumnFormat(columnNames.at(index), format);
    } else {
        qCritical() << tr("[PacketModelAbstract::setColumnFormat] Cannot find the column at (%1) T_T").arg(index);
    }
}

void PacketModelAbstract::setColumnFormat(const QString &columnName, OutputFormat format)
{
    if (columnName == COLUMN_PAYLOAD_STR) {
        Usercolumn uc = userColumnsDef.value(columnName);
        uc.format = format;
        userColumnsDef.insert(columnName, uc);
        emit dataChanged(createIndex(0,COLUMN_PAYLOAD), createIndex(size() - 1, COLUMN_PAYLOAD));
    } else if (userColumnsDef.contains(columnName)) {
        Usercolumn uc = userColumnsDef.value(columnName);
        uc.format = format;
        userColumnsDef.insert(columnName, uc);
        refreshColumn(columnName);
    } else {
        qCritical() << tr("[PacketModelAbstract::setColumnFormat] Cannot find the column name (%1) T_T").arg(columnName);
    }
}

OutputFormat PacketModelAbstract::getColumnFormat(int index)
{
    if ((index > COLUMN_COMMENT && index < columnNames.size()) || index == COLUMN_PAYLOAD) {
        return getColumnFormat(columnNames.at(index));
    } else {
        qCritical() << tr("[PacketModelAbstract::getColumnFormat] Cannot find the column at (%1) T_T").arg(index);
    }
    return Pip3lineConst::TEXTFORMAT;
}

OutputFormat PacketModelAbstract::getColumnFormat(const QString &columnName)
{
    if (userColumnsDef.contains(columnName)) {
        return userColumnsDef.value(columnName).format;
    } else {
        qCritical() << tr("[PacketModelAbstract::getColumnFormat] Cannot find the column name (%1) T_T").arg(columnName);
    }
    return Pip3lineConst::TEXTFORMAT;
}

QString PacketModelAbstract::getColumnName(int index)
{
    if (index >= 0 && index < columnNames.size()) {
        return columnNames.at(index);
    } else {
        qCritical() << tr("[PacketModelAbstract::getColumnName] Cannot find the column at (%1) T_T").arg(index);
    }
    return GuiConst::UNDEFINED_TEXT;
}