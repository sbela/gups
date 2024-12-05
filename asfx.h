#ifndef ASFX_H
#define ASFX_H

#include <cstring>
#include <iostream>
#include <memory>

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include <QDateTime>
#include <QFileInfo>
#include <QFontDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QRect>
#include <QScreen>
#include <QScroller>
#include <QSettings>

#include "aslogger.h"

#define SI static_cast<int32_t>

template <>
struct fmt::formatter<QByteArray>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const QByteArray& p, FormatContext& ctx) -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", p.toHex(' ').toStdString());
    }
};

template <>
struct fmt::formatter<QDate>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const QDate& p, FormatContext& ctx) -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", p.toString("yyyy.MM.dd").toStdString());
    }
};

template <>
struct fmt::formatter<QDateTime>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const QDateTime& p, FormatContext& ctx) -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", p.toString("yyyy.MM.dd hh:mm:ss.zzz").toStdString());
    }
};

template <>
struct fmt::formatter<QString>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const QString& p, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}", p.toStdString());
    }
};

template <>
struct fmt::formatter<QRect>
{
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.end();
    }

    template <typename FormatContext>
    auto format(const QRect p, FormatContext& ctx) -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "(x:{} y:{} w:{} h:{})", p.x(), p.y(), p.width(), p.height());
    }
};

struct format_string
{
    std::string_view str;
    format_string(const char* str)
        : str(str)
    {
    }
};

struct Globals
{
    static const QStringList tableHeaders;
    static ASLogger* pLog;
};

static void vPRINT(bool log, const char* format, fmt::format_args args)
{
    auto date { QDateTime::currentDateTime().toString(QStringLiteral("yy.MM.dd hh:mm:ss.zzz")).toStdString() };
    auto prompt  = fmt::format("{} # ", date);
    auto message = fmt::vformat(format, args);
    if (log and Globals::pLog)
        Globals::pLog->AddToLog(prompt + message);
    fmt::print("{}{}\n", prompt, message);
    std::fflush(stdout);
}

template <typename... Args>
void PRINT(const char* format, Args&&... args)
{
    vPRINT(true, format, fmt::make_format_args(args...));
}

template <typename... Args>
void PRINTNL(const char* format, Args&&... args)
{
    vPRINT(false, format, fmt::make_format_args(args...));
}
#endif // ASFX_H
