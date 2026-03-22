#include "TextModel.h"
#include <QSet>

void TextModel::setText(const QString &fullText)
{
    m_fullText = fullText;
    QString normalized = fullText;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    m_lines = normalized.split(QLatin1Char('\n'));
    if (!m_lines.isEmpty() && m_lines.last().isEmpty())
        m_lines.removeLast();
    if (m_lines.isEmpty())
        m_lines.append(QString());
    resetPosition();
}

void TextModel::resetPosition()
{
    m_lineIndex = 0;
    m_charIndex = 0;
    clearErrors();
}

bool TextModel::isFinished() const
{
    return m_lineIndex >= m_lines.size();
}

QString TextModel::lineAt(int i) const
{
    if (i < 0 || i >= m_lines.size()) return QString();
    return m_lines.at(i);
}

QString TextModel::currentLine()  const { return lineAt(m_lineIndex); }
QString TextModel::previousLine() const { return lineAt(m_lineIndex - 1); }

QString TextModel::donePart() const
{
    const QString line = currentLine();
    if (m_charIndex <= 0)           return QString();
    if (m_charIndex >= line.size()) return line;
    return line.left(m_charIndex);
}

QString TextModel::remainPart() const
{
    const QString line = currentLine();
    if (m_charIndex <= 0)           return line;
    if (m_charIndex >= line.size()) return QString();
    return line.mid(m_charIndex);
}

QString TextModel::currentChar() const
{
    const QString line = currentLine();
    if (m_charIndex < 0 || m_charIndex >= line.size()) return QString();
    return QString(line.at(m_charIndex));
}

bool TextModel::advance()
{
    if (isFinished()) return false;
    const int lineLen = currentLine().size();
    if (m_charIndex < lineLen)
        ++m_charIndex;
    if (m_charIndex >= lineLen) {
        ++m_lineIndex;
        m_charIndex = 0;
    }
    return true;
}

void TextModel::goBack()
{
    if (m_lineIndex == 0 && m_charIndex == 0) return; // already at start

    if (m_charIndex > 0) {
        --m_charIndex;
    } else if (m_lineIndex > 0) {
        --m_lineIndex;
        m_charIndex = currentLine().size(); // go to end-of-previous-line
        // Clamp to last char (not past it)
        if (m_charIndex > 0) --m_charIndex;
    }
}

void TextModel::markError()
{
    m_errors.insert(errorKey(m_lineIndex, m_charIndex));
}

bool TextModel::hasError(int line, int ch) const
{
    return m_errors.contains(errorKey(line, ch));
}

void TextModel::clearErrors()
{
    m_errors.clear();
}
