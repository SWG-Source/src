# Station id scheme

Without external authentication, the LoginServer turns an account name into a station id, and every character is stored under that id. The rule used to be `std::hash`, which gives different results on 32-bit and 64-bit builds. There are now two named, fixed rules:

- **gcc32**: the rule every 32-bit server used. New databases use it.
- **gcc64**: the rule used by servers built from the 64-bit branch before this was fixed. It is kept only so those databases keep working.

A database's accounts were created under one rule, so the **database** records which one, in the login table `station_id_scheme`. The LoginServer reads it at startup and will not start without it. The config key `[LoginServer] stationIdScheme` is optional and only double-checks the database; it cannot change the rule.

## New databases

`ant create_database` records `gcc32`. Nothing to do.

## Existing databases (after update 272)

Update 272 creates the table but leaves it empty, because only you or the data can say which rule your accounts were created with. Label it once:

1. Export the station ids from every schema that holds login or cluster tables (usually one schema). This only reads:

   ```
   sqlplus -s user/password@service @export.sql > ids.tsv
   ```

   With separate login and cluster schemas, run it in each and concatenate the outputs.

2. Let the tool decide from the evidence:

   ```
   stationIdScheme classify < ids.tsv > label.sql
   ```

   If `ids.tsv` is empty, `classify` will label the database as new, so first make sure you ran `export.sql` in the schema that holds `account_info` and `players`. An export from the wrong schema is empty.

   A character that has entered the world records its account name. For each station id, the tool checks which rule turns that name into that id. It records a label only when **every** station id is explained the same way.

3. If `classify` records nothing (the usual case for older servers, because characters from before 2022 and accounts without characters have no recorded name), state what you know:

   - **This server only ever ran 32-bit builds:** `stationIdScheme assert gcc32 < ids.tsv > label.sql`
   - **This server was created by the 64-bit branch before this fix:** `stationIdScheme assert gcc64 < ids.tsv > label.sql`

   The label is recorded as *asserted*, with the date. The tool refuses if any station id is proven to belong to the other rule.

4. Apply it (in the login schema):

   ```
   sqlplus -s user/password@service @label.sql
   ```

The report on stderr lists every station id the tool could not explain. Keep it with your records.

If `classify` reports that **both** rules are proven, accounts were created under both (for example, a server that ran 32-bit and then the unpatched 64-bit build). Those accounts cannot share one rule without re-keying one set of them. A verified re-key tool is planned separately; until then, restore from backup or resolve the accounts by hand.
