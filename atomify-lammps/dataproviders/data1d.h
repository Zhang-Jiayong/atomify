#ifndef DATA1D_H
#define DATA1D_H
#include <limits>
#include <QObject>
#include <QXYSeries>
#include <QLineSeries>

using namespace QtCharts;
class Data1D : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(float xMin READ xMin NOTIFY xMinChanged)
    Q_PROPERTY(float xMax READ xMax NOTIFY xMaxChanged)
    Q_PROPERTY(float yMin READ yMin NOTIFY yMinChanged)
    Q_PROPERTY(float yMax READ yMax NOTIFY yMaxChanged)
    Q_PROPERTY(int stride READ stride WRITE setStride NOTIFY strideChanged)
    Q_PROPERTY(Data1D* parentData READ parentData WRITE setParentData NOTIFY parentDataChanged)
    Q_PROPERTY(QVariantMap subsets READ subsets WRITE setSubsets NOTIFY subsetsChanged)
    Q_PROPERTY(QXYSeries* xySeries READ xySeries WRITE setXySeries NOTIFY xySeriesChanged)
    Q_PROPERTY(QVector<double> histogramPoints READ histogramPoints WRITE setHistogramPoints NOTIFY histogramPointsChanged)

public:
    explicit Data1D(QObject *parent = 0);
    Q_INVOKABLE void updateLimits();
    Q_INVOKABLE void updateXYSeries(QAbstractSeries *series);
    Q_INVOKABLE void updateHistogram(QLineSeries *series);
    Q_INVOKABLE void add(float x, float y, bool silent = true);
    Q_INVOKABLE void addSubset(QString key, int stride);
    Q_INVOKABLE void clear(bool silent = false);
    void createHistogram(const std::vector<double> &points);
    void add(const QPointF &point, bool silent = true);
    float xMin();
    float xMax();
    float yMin();
    float yMax();
    bool enabled() const;
    QVariantMap subsets() const;
    int stride() const;
    Data1D* parentData() const;
    QXYSeries* xySeries() const;
    QVector<double> histogramPoints() const;

signals:
    void xMinChanged(float xMin);
    void xMaxChanged(float xMax);
    void yMinChanged(float yMin);
    void yMaxChanged(float yMax);
    void minMaxChanged();
    void enabledChanged(bool enabled);
    void subsetsChanged(QVariantMap subsets);
    void strideChanged(int stride);
    void parentDataChanged(Data1D* parentData);
    void xySeriesChanged(QXYSeries* xySeries);
    void updated(Data1D *data);
    void updatedHistogram(Data1D *data);
    void histogramPointsChanged(QVector<double> histogramPoints);

public slots:
    void setEnabled(bool enabled);
    void setSubsets(QVariantMap subsets);
    void setStride(int stride);
    void setParentData(Data1D* parentData);
    void setXySeries(QXYSeries* xySeries);
    void setHistogramPoints(QVector<double> histogramPoints);

private:
    Data1D* m_parentData = nullptr;
    QXYSeries* m_xySeries = nullptr;
    QList<QPointF> m_points;
    std::vector<double> m_histogramPoints;
    QVector<double> m_cleanHistogramPoints;
    QVariantMap m_subsets;
    float m_xMin = 0;
    float m_xMax = 0;
    float m_yMin = 0;
    float m_yMax = 0;
    bool m_minMaxValuesDirty = false;
    bool m_enabled = false;
    int m_stride = 1;
    int m_strideCount = 0;
    void updateMinMaxWithPoint(const QPointF &point);
};

#endif // DATA1D_H
