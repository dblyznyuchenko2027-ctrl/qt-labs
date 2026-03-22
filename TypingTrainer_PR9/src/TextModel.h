#pragma once

#include <QString>
#include <QStringList>

// ─────────────────────────────────────────────────────────────────────────────
// TextModel — lesson text with cursor position and per-char error tracking.
//
// Invariants (always maintained):
//   0 <= lineIndex <= lines.size()   (== size means finished)
//   0 <= charIndex <= currentLine().size()
// ─────────────────────────────────────────────────────────────────────────────
class TextModel
{
public:
    TextModel() = default;

    // ── Load ─────────────────────────────────────────────────────────────────
    void setText(const QString &fullText);

    // ── Position ─────────────────────────────────────────────────────────────
    int  lineIndex() const { return m_lineIndex; }
    int  charIndex() const { return m_charIndex; }
    bool isFinished() const;

    // ── Lines ────────────────────────────────────────────────────────────────
    int         lineCount()    const { return m_lines.size(); }
    QString     lineAt(int i)  const;
    QString     currentLine()  const;
    QString     previousLine() const;

    // ── Fragments of current line ─────────────────────────────────────────────
    QString donePart()   const;   // line.left(charIndex)
    QString remainPart() const;   // line.mid(charIndex)
    QString currentChar() const;  // single char at charIndex, "" if at end

    // ── Navigation ───────────────────────────────────────────────────────────
    /// Advance one char (wraps to next line at end-of-line).
    /// Returns false if already finished.
    bool advance();

    /// Step back one char (within current line; stops at line start / text start).
    void goBack();

    void resetPosition();

    // ── Error tracking ───────────────────────────────────────────────────────
    /// Record that the char at (lineIndex, charIndex) was typed incorrectly.
    void markError();
    /// True if (lineIndex, charIndex) was previously marked as error.
    bool hasError(int line, int ch) const;
    /// Clear all error marks (call on resetPosition/new lesson).
    void clearErrors();

    // ── Raw data ─────────────────────────────────────────────────────────────
    QString     fullText() const { return m_fullText; }
    QStringList lines()    const { return m_lines; }

private:
    QString     m_fullText;
    QStringList m_lines;
    int         m_lineIndex = 0;
    int         m_charIndex = 0;

    // Flat set of error positions encoded as "line:char" strings
    QSet<QString> m_errors;

    static QString errorKey(int line, int ch)
    { return QString::number(line) + QLatin1Char(':') + QString::number(ch); }
};
