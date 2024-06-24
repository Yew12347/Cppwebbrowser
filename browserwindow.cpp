#include "browserwindow.h"
#include <QDebug>

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent), urlInput(nullptr), goButton(nullptr), htmlDisplay(nullptr)
{
    setupUI();
    connectSignals();
}

void BrowserWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    urlInput = new QLineEdit(this);
    QPushButton *goButton = new QPushButton("Go", this); // Updated to local variable

    htmlDisplay = new QTextEdit(this);
    htmlDisplay->setReadOnly(true);

    QHBoxLayout *urlLayout = new QHBoxLayout();
    urlLayout->addWidget(urlInput);
    urlLayout->addWidget(goButton);

    layout->addLayout(urlLayout);
    layout->addWidget(htmlDisplay);

    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

void BrowserWindow::connectSignals() {
    connect(urlInput, &QLineEdit::returnPressed, this, &BrowserWindow::loadUrl);
    connect(goButton, &QPushButton::clicked, this, &BrowserWindow::loadUrl);
}

void BrowserWindow::loadUrl() {
    QString url = urlInput->text();
    if (!url.startsWith("http://") && !url.startsWith("https://")) {
        url = "http://" + url; // Ensure URL starts with protocol
    }
    fetchUrl(url);
}

void BrowserWindow::fetchUrl(const QString &url) {
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onUrlFetched(reply);
    });
}

void BrowserWindow::onUrlFetched(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString htmlContent = QString::fromUtf8(reply->readAll());
        processAndDisplayHtml(htmlContent);
    } else {
        displayError(reply->errorString());
    }
    reply->deleteLater();
}

void BrowserWindow::processAndDisplayHtml(const QString &html) {
    QString processedHtml = applyBasicCss(html);
    processedHtml = executeJavaScript(processedHtml);
    htmlDisplay->setHtml(processedHtml);
}

QString BrowserWindow::applyBasicCss(const QString &html) {
    QString processedHtml = html;

    // Extract CSS from <style> tags
    QRegularExpression styleRegex("<style[^>]*>(.*?)</style>", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator matchIterator = styleRegex.globalMatch(html);

    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString styleContent = match.captured(1);
        applyCss(styleContent, processedHtml);
        processedHtml.remove(match.capturedStart(0), match.capturedLength(0)); // Remove <style> tag
    }

    return processedHtml;
}

void BrowserWindow::applyCss(const QString &cssContent, QString &html) {
    // Initialize libcss
    css_error error;
    css_stylesheet *sheet;
    error = css_stylesheet_create(CSS_LEVEL_DEFAULT, "UTF-8", NULL, NULL, NULL, NULL, &sheet);
    if (error != CSS_OK) {
        qWarning() << "Failed to create stylesheet";
        return;
    }

    // Parse the CSS content
    error = css_stylesheet_append_data(sheet, cssContent.toUtf8().data(), cssContent.toUtf8().size());
    if (error != CSS_OK) {
        qWarning() << "Failed to append data to stylesheet";
        css_stylesheet_destroy(sheet);
        return;
    }

    error = css_stylesheet_data_done(sheet);
    if (error != CSS_OK) {
        qWarning() << "Failed to finalize stylesheet";
        css_stylesheet_destroy(sheet);
        return;
    }

    // Apply the stylesheet to your HTML content
    // This is a simplified example of applying inline styles
    QRegularExpression pTagRegex("<p[^>]*>");
    QRegularExpressionMatchIterator matchIterator = pTagRegex.globalMatch(html);

    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString pTag = match.captured(0);
        QString styledPTag = pTag;
        if (!pTag.contains("style=")) {
            styledPTag.insert(pTag.length() - 1, " style='color:blue;'");
        }
        html.replace(pTag, styledPTag);
    }

    // Clean up
    css_stylesheet_destroy(sheet);
}

QString BrowserWindow::executeJavaScript(const QString &html) {
    QJSEngine jsEngine;
    QRegularExpression scriptRegex("<script[^>]*>(.*?)</script>", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator matchIterator = scriptRegex.globalMatch(html);
    QString modifiedHtml = html;

    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        QString scriptContent = match.captured(1);

        // Evaluate the JavaScript code
        QJSValue result = jsEngine.evaluate(scriptContent);

        if (result.isError()) {
            qWarning() << "Uncaught exception at line"
                       << result.property("lineNumber").toInt() << ":"
                       << result.toString();
        } else {
            // Optionally process the result and modify HTML accordingly
            // Here, simply remove the script tag after execution
            modifiedHtml.remove(match.capturedStart(0), match.capturedLength(0));
        }
    }

    return modifiedHtml;
}

void BrowserWindow::displayError(const QString &error) {
    htmlDisplay->setHtml("<p style='color:red;'><strong>Error:</strong> " + error + "</p>");
}
