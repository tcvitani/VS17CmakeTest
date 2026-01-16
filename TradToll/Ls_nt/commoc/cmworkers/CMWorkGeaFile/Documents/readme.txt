1.0.0 19.12.2003 TCV

   - Module creation

1.0.1 4.2.2004 TCV

   - Decoding of the new payment types has been added. The following GEA payment
     types has been converted to CSSI freeway payment types:
          - monthly subscription
          - annual subscription
          - HAC vehicles
          - HAC permission
          - disabled persons
          - Croatian army

1.0.2 23.2.2004 TCV

   - Decoding of the license plate number has been added.

1.0.3 30.03.2004 TCV

   - The new exchange rate has been decoded from the transaction message. This
     version is unsuccessfully installed on site. We have discovered that other
     GEA transaction message fields has been changed too (Toll fare, amount paid
     in foreign currency, partial fare etc.).

1.0.4 01.04.2004 TCV

   - In the new version of the GEA transaction message all the amounts are multiplied
     by 100 (in the old transaction message all the amounts have been multiplied by 10). The
     new version of the conversion module detects the transaction version and makes the
     appropriate conversion.
   - The End of Job message has been changed too. The new message contains the TYP counter
     that is positioned on the place different from the old version of the GEA End of Job
     message. In addition, the new counter is multiplied by 100 (in the old message it was 
     multiplied by 10). The new GEA End of Job message contains the new header (header type II).

1.0.5 15.04.2004 TCV

   - Decoding of the entry ticket number has been added. The entry ticket number is
     stored in the transaction complementary info message (code 9,1).

1.0.6 28.04.2004 TCV

1.0.7 13.09.2004 TCV

   - There was a problem to decode transactions that have the toll fare zero. The module
     detected these type of transactions as anomalies. Since in the HAC III project 
     the toll fare for some routes is zero the transactions have not been imported into
     database. The problem has been solved. All the transaction that has the billing code 
     different from zero and the transaction code equal to 70 or 77 are decoded as real
     transactions even if the toll fare is set to zero.
   - In the transaction conversation function the entry lane number was decoded from the first
     transaction item. In the case when the toll officer cancels the transaction and keys in
     the plaza number different from the plaza number read from the entry ticket the decoding is
     wrong. In this case it is possible to generate the trasanction that can't be inserted into
     the database because it contains the entry lane number that doesn't exist for the plaza
     keyed in by the toll officer. The conversion function has been changed to take the entry
     lane number from the last GEA transaction item. If the toll officer cancells the transaction
     and keys in the plaza ID the entry lane number will be set to zero.

1.0.8 27.09.2004 TCV

   - The maximal number of stored GEA transaction items (that waits in the memory to be
     converted) has been increased from 10 to 100. Transactions that contain more that 100 
     anomalies will be ignored.

1.0.9 07.10.2004 TCV

  - GEA has added the new field inside the transaction message that represents the network
    number of the keyed in entry plaza. In the previous GEA transaction messages the TAG network
    ID was set to zero if the toll officer keys in the plaza number. The CS service was decoding
    the entry plaza ID (three digits) by using the network ID from the transaction header. It couldn't
    work correctly because the network ID from the transaction header is exit plaza network ID. The
    conversion module has been changed to use the new field from the GEA transaction message to
    decode the entry plaza ID.
  - The conversion of the prepaid payment means was not tested before because the functionality
    was not implemented during the test period. After GEA has made some test transactions on site
    we have discovered that the transactions where not converted and inserted into database. It
    is because we have supposed that prepaid transactions (CSC and TAG) have partial fare set to zero.
    In the GEA data files we have found that even the transaction that have billing code set to zero
    can have the partial fare different from zero (if the toll officer cancels the prepaid transaction).
    The module has been adapted to correctly convert the transaction made by prepaid payment menas. In
    addition the partial fare in not used in the transaction conversion if the billing code is set to zero.

1.1.0 02.12.2004 TCV

  - In the case of U-turn the module has used the entry lane ID and entry plaza ID from the
    GEA trasnasction to fill the CSSI transaction. This type of transaction couldn't be inserted
    in the database because U-turn should be inserted as a specific plaza ID (96 or 97) that doesn't have
    any entry lane defined. The only acceptable lane ID for this type of transaction is zero. The module is
    addapted to fill entry lane ID (in the entry information structure) with zero when it detects the 
    plaza ID 96 or 97.
  - The payment message for the Tip payment mean was marked as invalid payment so this type of transaction
    did not increase the revenue counters. The module is addapted to set the payment valid flag (in the payment
    message) to one when it detects the Tip transaction type.

1.1.1 21.10.2005 TCV

  - The new Exempt Island payment type has been added. The module converts the new exempt from GEA billing
    code 8 or 27 and observation code 62 to CS payment type 2 and payment subtype 15.

1.1.2 13.09.2006 TCV

  - In the automatic mode HAC has to make reconciliation to create specific statistics based on the accounting
    day. The SOS message conversion has been addapted to set the exploitation mode to 1 (manual) when the
    automatic mode is detected in the GEA SOS message.