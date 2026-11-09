# Station id scheme

Without external authentication, the LoginServer turns an account name into a station id, and every character is stored under that id. The rule used to be `std::hash`, whose result is implementation-defined: it can change with the compiler, the standard library version or the platform. In practice it gave different ids on the 32-bit and 64-bit GCC builds. There are now two named, fixed rules that no longer depend on the compiler:

- **gcc32**: the rule every 32-bit server used. New databases use it.
- **gcc64**: the rule used by servers built from the 64-bit branch before this was fixed. It is kept only so those databases keep working.

A database's accounts were created under one rule, so the **database** records which one, in the login table `station_id_scheme`. The LoginServer reads it at startup and will not start without it. The config key `[LoginServer] stationIdScheme` is optional and only double-checks the database; it cannot change the rule.

## New databases

`ant create_database` records `gcc32`. Nothing to do.

## Existing databases

Update 272 creates the table but leaves it empty. swg-main labels it for you: `ant update_database` applies the update and then records the scheme, and `ant compile` does the same check. To run only the labelling step:

```
ant label_station_id_scheme
```

It exports the station ids and lets the `stationIdScheme` tool decide from the evidence. When the evidence is incomplete (the usual case for older servers, because characters from before 2022 and accounts without characters have no recorded name), it records `station_id_scheme` from `build.properties`, which is `gcc32`: the rule every 32-bit server used. The label is then marked *asserted*, with the date. Running it on a database that is already labelled does nothing.

The tool refuses `gcc32` if any account proves the other rule, and the build stops. A database created by the 64-bit branch before this fix sets `station_id_scheme = gcc64` in `local.properties` before running it.

The report is kept in `build/station_id_scheme/report.txt`. It lists every station id the tool could not explain. Keep it with your records.

If the tool reports that **both** rules are proven, accounts were created under both (for example, a server that ran 32-bit and then the unpatched 64-bit build). Those accounts cannot share one rule without re-keying one set of them. A verified re-key tool is planned separately; until then, restore from backup or resolve the accounts by hand.

## By hand

The Ant target runs these steps. Run them yourself if your login and cluster tables live in different schemas. Replace `user`, `password`, `host` and `service` with the values from `build.properties` (`db_username`, `db_password`, `dbip`, `db_service`). `stationIdScheme` is built into `build/bin` in swg-main (also reachable as `exe/linux/bin`).

1. Export the station ids. Run this in this directory; it only reads:

   ```
   sqlplus -s -L user/password@host/service @export.sql > ids.tsv
   ```

   The connect string needs the host as well as the service (`@localhost/swg`, not `@swg`); without the host, Oracle answers ORA-12504. With separate login and cluster schemas, run it in each and concatenate the outputs.

2. Let the tool decide:

   ```
   ~/swg-main/build/bin/stationIdScheme classify < ids.tsv > label.sql
   ```

   A character that has entered the world records its account name. For each station id, the tool checks which rule turns that name into that id. It records a label only when **every** station id is explained the same way. Otherwise it writes nothing, and the shell leaves `label.sql` empty. In that case, state what you know instead:

   ```
   ~/swg-main/build/bin/stationIdScheme assert gcc32 < ids.tsv > label.sql
   ```

   If `ids.tsv` is empty, `classify` labels the database as new. Make sure you ran `export.sql` in the schema that holds `account_info` and `players`; an export from the wrong schema is empty.

3. Apply it (in the login schema), only if `label.sql` is not empty:

   ```
   sqlplus -s -L user/password@host/service @label.sql
   ```
