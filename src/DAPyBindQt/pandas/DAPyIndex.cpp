﻿#include "DAPyIndex.h"
#include "DAPyModulePandas.h"
#include "DAPybind11QtTypeCast.h"
//===================================================
// using DA namespace -- 禁止在头文件using！！
//===================================================

using namespace DA;

//===================================================
// DAPyIndex
//===================================================
DAPyIndex::DAPyIndex() : DAPyObjectWrapper()
{
	try {
		auto pandas = DAPyModule("pandas");
		_object     = pandas.attr("Index");
	} catch (const std::exception& e) {
		qCritical() << "can not import pandas,or can not create pandas.Index(),because:" << e.what();
	}
}

DAPyIndex::DAPyIndex(const DAPyIndex& s) : DAPyObjectWrapper(s)
{
	checkObjectValid();
}

DAPyIndex::DAPyIndex(DAPyIndex&& s) : DAPyObjectWrapper(std::move(s))
{
	checkObjectValid();
}

DAPyIndex::DAPyIndex(const pybind11::object& obj) : DAPyObjectWrapper(obj)
{
	checkObjectValid();
}

DAPyIndex::DAPyIndex(pybind11::object&& obj) : DAPyObjectWrapper(std::move(obj))
{
	checkObjectValid();
}

DAPyIndex::~DAPyIndex()
{
}

bool DAPyIndex::isIndexObj(const pybind11::object& obj)
{
	return DAPyModulePandas::getInstance().isInstanceIndex(obj);
}

DAPyIndex& DAPyIndex::operator=(const pybind11::object& obj)
{
	_object = obj;
	checkObjectValid();
	return *this;
}

DAPyIndex& DAPyIndex::operator=(pybind11::object&& obj)
{
	_object = std::move(obj);
	checkObjectValid();
	return *this;
}

DAPyIndex& DAPyIndex::operator=(const DAPyIndex& obj)
{
	if (this != &obj) {
		DAPyObjectWrapper::operator=(obj);  // 调用基类赋值
		checkObjectValid();
	}
	return *this;
}

DAPyIndex& DAPyIndex::operator=(DAPyIndex&& obj)
{
	if (this != &obj) {
		DAPyObjectWrapper::operator=(std::move(obj));  // 调用基类移动赋值
		checkObjectValid();
	}
	return *this;
}

DAPyIndex& DAPyIndex::operator=(const DAPyObjectWrapper& obj)
{
	DAPyObjectWrapper::operator=(obj);
	checkObjectValid();
	return *this;
}

DAPyIndex& DAPyIndex::operator=(DAPyObjectWrapper&& obj)
{
	DAPyObjectWrapper::operator=(std::move(obj));
	checkObjectValid();
	return *this;
}

QVariant DAPyIndex::operator[](std::size_t i) const
{
	try {
		pybind11::object obj = object()[ pybind11::int_(i) ];
		return DA::PY::toVariant(obj);
	} catch (const std::exception& e) {
		qCritical().noquote() << e.what();
	}
	return QVariant();
}

pybind11::object DAPyIndex::operator[](const QSet< std::size_t >& slice) const
{
	try {
		return object()[ DA::PY::toPyList(slice) ];
	} catch (const std::exception& e) {
		qCritical().noquote() << e.what();
	}
	return pybind11::none();
}

/**
 * @brief Return the dtype object of the underlying data.
 * @return
 */
pybind11::dtype DAPyIndex::dtype() const
{
	try {
		return object().attr("dtype");
	} catch (const std::exception& e) {
		qCritical().noquote() << e.what();
	}
	return pybind11::none();
}

bool DAPyIndex::empty() const
{
	try {
		pybind11::bool_ obj = object().attr("empty");
		return obj.cast< bool >();
	} catch (const std::exception& e) {
		qCritical().noquote() << e.what();
	}
	return true;
}

std::size_t DAPyIndex::size() const
{
	try {
		pybind11::int_ obj = object().attr("size");
		return std::size_t(obj);
	} catch (const std::exception& e) {
		qCritical().noquote() << e.what();
	}
	return 0;
}

void DAPyIndex::checkObjectValid()
{
	if (!isIndexObj(object())) {
		object() = pybind11::none();
		qCritical() << QObject::tr("DAPyIndex get python object type is not pandas.Index");
	}
}
