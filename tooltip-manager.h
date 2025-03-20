#ifndef TOOLTIP_MANAGER_H
#define TOOLTIP_MANAGER_H

#include <QLabel>
#include <QObject>

class ToolTipManager;
class TipWidget : public QLabel
{
    Q_OBJECT
    friend class ToolTipManager;
public:
    explicit TipWidget(const QString& msg, QWidget* parent=nullptr);

private:
    qint32                              mCurrentSec = 0;
    qint32                              mTotalSec;
};

class ToolTipManager : public QWidget
{
    Q_OBJECT
public:
    static ToolTipManager* getInstance();

    void showMessage(QString msg);

private:
    ~ToolTipManager();
    explicit ToolTipManager(const ToolTipManager& tp) {};
    explicit ToolTipManager(QWidget* parent = nullptr);

private Q_SLOTS:
    void checkTipWidgets();

protected:
    void showEvent(QShowEvent *event);

private:
    qint32                              mMaxTip;
    static ToolTipManager*              gInstance;
    QRect                               mMainPos;
    QTimer*                             mMainTimer = nullptr;
    QLayout*                            mMainLayout = nullptr;
    QList<std::shared_ptr<TipWidget>>   mMainWidgets;
};


#endif // TOOLTIP-MANAGER_H
