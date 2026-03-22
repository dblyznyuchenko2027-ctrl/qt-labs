#pragma once

#include <QString>
#include <QStringList>

// ─────────────────────────────────────────────────────────────────────────────
// TextModel — holds lesson text, split into lines, with cursor position.
//
// Invariants (always maintained after any public call):
//   0 <= lineIndex < lines.size()   (or lineIndex == 0 when lines is empty)
//   0 <= charIndex <= currentLine().size()
// ─────────────────────────────────────────────────────────────────────────────
class TextModel
{
public:
    TextModel() = default;

    // ── Load ─────────────────────────────────────────────────────────────────
    /// Set new full text; splits on '\n'; resets position to start.
    void setText(const QString &fullText);

    // ── Position accessors ───────────────────────────────────────────────────
    int  lineIndex() const { return m_lineIndex; }
    int  charIndex() const { return m_charIndex; }
    bool isFinished() const;

    // ── Line accessors ───────────────────────────────────────────────────────
    int         lineCount()  const { return m_lines.size(); }
    QString     lineAt(int i) const;          // safe — returns "" if out of range
    QString     currentLine() const;
    QString     previousLine() const;         // "" when on first line

    // ── Position fragments ───────────────────────────────────────────────────
    /// Part of currentLine() already passed: line.left(charIndex)
    QString donePart()    const;
    /// Remaining part of currentLine() from charIndex onwards: line.mid(charIndex)
    QString remainPart()  const;
    /// The single character at charIndex (current target char), or "" at end-of-line
    QString currentChar() const;

    // ── Navigation ───────────────────────────────────────────────────────────
    /// Advance one character; wraps to next line when line ends.
    /// Returns false if already finished (no move made).
    bool advance();

    /// Reset position to beginning of text.
    void resetPosition();

    // ── Raw data ─────────────────────────────────────────────────────────────
    QString     fullText()  const { return m_fullText; }
    QStringList lines()     const { return m_lines; }

private:
    void clamp();   // ensure indices stay in bounds

    QString     m_fullText;
    QStringList m_lines;
    int         m_lineIndex = 0;
    int         m_charIndex = 0;
};
