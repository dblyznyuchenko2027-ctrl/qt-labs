#include "TextModel.h"

// ─────────────────────────────────────────────────────────────────────────────
void TextModel::setText(const QString &fullText)
{
    m_fullText = fullText;

    // Normalize line endings then split
    QString normalized = fullText;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    m_lines = normalized.split(QLatin1Char('\n'));

    // Remove trailing empty line that split() often produces
    if (!m_lines.isEmpty() && m_lines.last().isEmpty())
        m_lines.removeLast();

    // Guard: always have at least one (possibly empty) line
    if (m_lines.isEmpty())
        m_lines.append(QString());

    resetPosition();
}

// ─────────────────────────────────────────────────────────────────────────────
void TextModel::resetPosition()
{
    m_lineIndex = 0;
    m_charIndex = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
bool TextModel::isFinished() const
{
    if (m_lines.isEmpty()) return true;
    // Finished when we are past the last line
    return m_lineIndex >= m_lines.size();
}

// ─────────────────────────────────────────────────────────────────────────────
QString TextModel::lineAt(int i) const
{
    if (i < 0 || i >= m_lines.size())
        return QString();
    return m_lines.at(i);
}

QString TextModel::currentLine() const
{
    return lineAt(m_lineIndex);
}

QString TextModel::previousLine() const
{
    return lineAt(m_lineIndex - 1);   // lineAt handles negative index safely
}

// ─────────────────────────────────────────────────────────────────────────────
QString TextModel::donePart() const
{
    const QString line = currentLine();
    if (m_charIndex <= 0)            return QString();
    if (m_charIndex >= line.size())  return line;
    return line.left(m_charIndex);
}

QString TextModel::remainPart() const
{
    const QString line = currentLine();
    if (m_charIndex <= 0)            return line;
    if (m_charIndex >= line.size())  return QString();
    return line.mid(m_charIndex);
}

QString TextModel::currentChar() const
{
    const QString line = currentLine();
    if (m_charIndex < 0 || m_charIndex >= line.size())
        return QString();
    return QString(line.at(m_charIndex));
}

// ─────────────────────────────────────────────────────────────────────────────
bool TextModel::advance()
{
    if (isFinished()) return false;

    const int lineLen = currentLine().size();

    if (m_charIndex < lineLen) {
        // Still characters left on this line
        ++m_charIndex;
    }

    // Move to next line when we have consumed all chars (including end-of-line)
    if (m_charIndex >= lineLen) {
        ++m_lineIndex;
        m_charIndex = 0;
    }

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
void TextModel::clamp()
{
    if (m_lines.isEmpty()) {
        m_lineIndex = 0;
        m_charIndex = 0;
        return;
    }
    m_lineIndex = qBound(0, m_lineIndex, m_lines.size());   // may equal size → finished
    if (m_lineIndex < m_lines.size()) {
        m_charIndex = qBound(0, m_charIndex, m_lines.at(m_lineIndex).size());
    } else {
        m_charIndex = 0;
    }
}
