#ifndef TOOLTIP_MANAGER_H
#define TOOLTIP_MANAGER_H

#include <QLabel>
#include <QObject>

class ToolTipManager;
class TipWidget : public QLabel
{
    Q_OBJECT
public:
    explicit TipWidget(const QString& msg, QWidget* parent=nullptr);
};

class TipWrap : public QWidget
{
    Q_OBJECT
    friend class ToolTipManager;
public:
    explicit TipWrap(const QString& msg, QWidget* parent=nullptr);

Q_SIGNALS:
    void closeTip();


private:
    TipWidget*                          mWidget = nullptr;
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
    void closeTip(std::shared_ptr<TipWrap> tip);
    void checkTipWidgets();

protected:
    void showEvent(QShowEvent *event);

private:
    qint32                              mMaxTip;
    static ToolTipManager*              gInstance;
    QRect                               mMainPos;
    QTimer*                             mMainTimer = nullptr;
    QLayout*                            mMainLayout = nullptr;
    QList<std::shared_ptr<TipWrap>>     mMainWidgets;
};


#endif // TOOLTIP-MANAGER_H
