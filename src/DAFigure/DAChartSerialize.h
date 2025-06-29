﻿#ifndef DACHARTSERIALIZE_H
#define DACHARTSERIALIZE_H
#include "DAFigureAPI.h"
#include <string>
#include <QDataStream>
#include <functional>
#include "qwt_text.h"
#include "qwt_samples.h"
// DAChart
#include "DAChartPlotItemFactory.h"

class QwtPlotItem;
class QwtSymbol;
class QwtPlotCanvas;
class QFrame;
class QwtScaleWidget;
class QwtPlot;
class QwtColorMap;
class QwtLinearColorMap;
class QwtAlphaColorMap;
class QwtHueColorMap;
class QwtSaturationValueColorMap;
class QwtScaleDraw;
class QwtColumnSymbol;
class QwtIntervalSymbol;
// QwtPlotItem
class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlotScaleItem;
class QwtPlotLegendItem;
class QwtPlotMarker;
class QwtPlotSpectroCurve;
class QwtPlotIntervalCurve;
class QwtPlotHistogram;
class QwtPlotSpectrogram;
class QwtPlotGraphicItem;
class QwtPlotTradingCurve;
class QwtPlotBarChart;
class QwtPlotMultiBarChart;
class QwtPlotShapeItem;
class QwtPlotTextLabel;
class QwtPlotZoneItem;
class QwtPlotVectorField;

///
/// \brief 序列化类都是带异常的，使用中需要处理异常
///
namespace DA
{
///< 版本标示，每个序列化都应该带有版本信息，用于对下兼容
const int gc_dachart_version                     = 1;
const std::uint32_t gc_dachart_magic_mark        = 0x5A6B4CF1;
const std::uint32_t gc_dachart_magic_mark2       = 0xAA123456;
const std::uint32_t gc_dachart_magic_mark3       = 0x12345678;
const std::uint32_t gc_dachart_magic_mark4       = 0xAAB23498;
const QDataStream::Version gc_datastream_version = QDataStream::Qt_5_12;

class DAFIGURE_API DABadSerializeExpection : public std::exception
{
public:
	DABadSerializeExpection();
	DABadSerializeExpection(const char* why);
	DABadSerializeExpection(const std::string& why);
	~DABadSerializeExpection();
	const char* what() const noexcept;

private:
	std::string mWhy;
};

/**
 * @brief 针对QwtPlotItem的二进制序列化
 */
class DAFIGURE_API DAChartItemSerialize
{
public:
	class Header
	{
	public:
		Header();
		~Header();
		std::uint32_t magic;       ///< 魔数，--4
		int version;               ///< 版本 -- 8
		int rtti;                  ///< rtti --12
		unsigned char byte[ 20 ];  ///< 预留20字节，凑齐32字节
		// 是否正确的header
		bool isValid() const;

		friend QDataStream& operator<<(QDataStream& out, const DA::DAChartItemSerialize::Header& f);
		friend QDataStream& operator>>(QDataStream& in, DA::DAChartItemSerialize::Header& f);
	};

public:
	/**
	 * @brief 序列化为QByteArray函数
	 */
	using FpSerializeOut = std::function< QByteArray(const QwtPlotItem*) >;  // QByteArray serializeOut(QwtPlotItem* item)
	using FpSerializeIn =
		std::function< QwtPlotItem*(const QByteArray&) >;  // QwtPlotItem* serializeIn(int rtti,const QByteArray& d)
public:
	DAChartItemSerialize();
	~DAChartItemSerialize();
	/**
	 * @brief 注册序列化函数
	 * @param rtti item的rtti
	 * @param fp 函数指针
	 */
	static void registSerializeFun(int rtti, FpSerializeIn fpIn, FpSerializeOut fpOut);

	/**
	 * @brief 判断这个rtti是否支持序列化
	 * @param rtti
	 * @return
	 */
	static bool isSupportSerialize(int rtti);

	/**
	 * @brief 获取Serialize In方法
	 * @param rtti
	 * @return
	 */
	static FpSerializeIn getSerializeInFun(int rtti) noexcept;

	/**
	 * @brief 获取Serialize Out方法
	 * @param rtti
	 * @return
	 */
	static FpSerializeOut getSerializeOutFun(int rtti);
	/**
	 * @brief 序列化输出
	 * @param item
	 * @return
	 */
	QByteArray serializeOut(const QwtPlotItem* item) const;
	QwtPlotItem* serializeIn(const QByteArray& byte) const noexcept;

	/**
	 * @brief 在文件中获取rtti
	 *
	 * 文件的rtti保存在文件头中
	 * @param byte 文件内存
	 * @return
	 */
	int getRtti(const QByteArray& byte) const noexcept;

public:
	//
	/**
	 * @brief 模板化的序列化实现
	 *
	 * 对于QwtPlotItem实例化类的序列号，你只需要实现
	 * @code
	 * QDataStream& operator<<(QDataStream& out, const QwtPlotXXXItem* item);
	 * QDataStream& operator>>(QDataStream& in, QwtPlotXXXItem* item);
	 * @endcode
	 *
	 * 这两个函数即可，然后在cpp文件中显示实例化serializeIn_T和serializeOut_T
	 * @code
	 * template QwtPlotItem* DAChartItemSerialize::serializeIn_T< QwtPlotXXXItem, QwtPlotItem::Rtti_XXX >(const
	 *QByteArray&); template QByteArray DAChartItemSerialize::serializeOut_T< QwtPlotXXXItem >(const QwtPlotItem*);
	 * @endcode
	 *
	 * 再通过DAChartItemSerialize::registSerializeFun注册
	 *
	 * @code
	 * DAChartItemSerialize::registSerializeFun(QwtPlotItem::Rtti_XXX
	 *	   ,&DAChartItemSerialize::serializeIn_T< QwtPlotXXXItem, QwtPlotItem::Rtti_XXX >
	 *     ,&DAChartItemSerialize::serializeOut_T< QwtPlotXXXItem >);
	 * @endcode
	 *
	 * 这样，调用DAChartItemSerialize::serializeIn/serializeOut函数就能对item进行序列化
	 *
	 * @param byte
	 * @return
	 */
	template< typename T, int RTTI >
	static QwtPlotItem* serializeIn_T(const QByteArray& byte)
	{
		return deserializeImpl< T >(byte, RTTI);
	}

	template< typename T >
	static QByteArray serializeOut_T(const QwtPlotItem* item)
	{
		return serializeImpl< T >(item);
	}

private:
	// 序列化/反序列化的通用实现
	template< typename T >
	static QwtPlotItem* deserializeImpl(const QByteArray& byte, int expectedRtti)
	{
		QDataStream st(byte);
		st.setVersion(gc_datastream_version);

		Header h;
		st >> h;
		if (!h.isValid() || expectedRtti != h.rtti) {
			return nullptr;
		}

		QwtPlotItem* item = DAChartPlotItemFactory::createItem(expectedRtti);
		if (item) {
			st >> static_cast< T* >(item);
		}
		return item;
	}

	template< typename T >
	static QByteArray serializeImpl(const QwtPlotItem* item)
	{
		QByteArray byte;
		QDataStream st(&byte, QIODevice::WriteOnly);
		st.setVersion(gc_datastream_version);

		Header h;
		h.rtti = item->rtti();
		st << h << static_cast< const T* >(item);
		return byte;
	}

protected:
	static QHash< int, std::pair< FpSerializeIn, FpSerializeOut > >& serializeFun();
};

}  // end namespace DA

// 由于Qwt都在全局命名空间，按照ADL原则，编译器会在全局命名空间找对应的序列化函数

// QFrame的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QFrame* f);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QFrame* f);

// QwtText的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtText& t);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtText& t);
// QwtSymbol的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtSymbol* t);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtSymbol* t);
// QwtIntervalSymbol的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtIntervalSymbol* t);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtIntervalSymbol* t);
// QwtColumnSymbol的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtColumnSymbol* t);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtColumnSymbol* t);

// QwtScaleWidget指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtScaleWidget* w);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtScaleWidget* w);
// QwtScaleDraw指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtScaleDraw* w);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtScaleDraw* w);

/// @group QwtPlotItem相关
/// @{
// QwtPlotItem指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotItem* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotItem* item);
// QwtPlotCurve指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotCurve* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotCurve* item);
// QwtPlotGrid(Rtti_PlotGrid)指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotGrid* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotGrid* item);
// QwtPlotScaleItem(Rtti_PlotScale)指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotScaleItem* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotScaleItem* item);
// QwtPlotLegendItem(Rtti_PlotLegend)指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotLegendItem* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotLegendItem* item);
// QwtPlotMarker(Rtti_PlotMarker)指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotMarker* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotMarker* item);
// QwtPlotSpectroCurve(Rtti_PlotSpectroCurve)指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotSpectroCurve* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotSpectroCurve* item);
// QwtPlotBarChart指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotBarChart* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotBarChart* item);
// QwtPlotIntervalCurve指针的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotIntervalCurve* item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotIntervalCurve* item);

/// @}

// QwtPlotCanvas的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlotCanvas* c);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlotCanvas* c);
// QwtColorMap的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtColorMap* c);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtColorMap* c);
// QwtLinearColorMap的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtLinearColorMap* c);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtLinearColorMap* c);
// QwtAlphaColorMap的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtAlphaColorMap* c);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtAlphaColorMap* c);
// QwtHueColorMap的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtHueColorMap* c);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtHueColorMap* c);
// QwtSaturationValueColorMap的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtSaturationValueColorMap* c);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtSaturationValueColorMap* c);
// QwtPlot的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtPlot* chart);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtPlot* chart);

// QwtIntervalSample的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtIntervalSample& item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtIntervalSample& item);
// QwtInterval的序列化
DAFIGURE_API QDataStream& operator<<(QDataStream& out, const QwtInterval& item);
DAFIGURE_API QDataStream& operator>>(QDataStream& in, QwtInterval& item);

// 下面这两个要放到DA命名空间外，因为使用了QVector<T>的<<

#endif  // SAQWTSERIALIZE_H
