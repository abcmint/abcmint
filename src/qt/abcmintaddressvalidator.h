#ifndef ABCMINTADDRESSVALIDATOR_H
#define ABCMINTADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator.
   Corrects near-miss characters and refuses characters that are not part of base58.
 */
class AbcmintAddressValidator : public QValidator
{
    Q_OBJECT

public:
    explicit AbcmintAddressValidator(QObject *parent = 0);

    State validate(QString &input, int &pos) const;

    static const int MaxAddressLength = 49;
};

#endif // ABCMINTADDRESSVALIDATOR_H
