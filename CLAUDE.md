# TempleOS — Claude project instructions

## Token / Cost Ledger (billing-grade)

The owner bills clients from `LEDGER.md` at this repo's root. Entries must be **exact, not estimated**.

### How to record a commit

After every substantive commit you author:

```bash
~/.claude/billing/ledger.py --append --summary "<short description>"
git add LEDGER.md
git commit -m "chore(ledger): $(git rev-parse --short HEAD~0)"
```

The script:

- Reads token usage from the JSONL transcripts under `~/.claude/projects/<encoded-cwd>/`.
- Sums assistant turns in the window `(last ledger row's date, HEAD commit time]`, normalized to UTC.
- Prices each turn against `~/.claude/billing/pricing.json` (per-model, per-tier, long-context aware, service-tier aware, server-tool aware).
- **Aborts with exit code 3 if it encounters an unknown model or service tier.** A missing row is fine; a wrong row is not.
- Exits 2 with no row if no usage was found in the window (e.g. a human-only commit).

Use `--dry-run` to preview without writing. Use `--commit <sha>` to ledger a specific commit rather than HEAD. See `~/.claude/billing/ledger.py --help` for all flags.

### Rules

- **Never bypass the script.** Do not hand-author rows, estimate tokens, or fabricate costs. If the script can't produce a row (unknown model, missing transcript, session ran from a different cwd), stop and surface the problem to the owner.
- **Append-only.** Never rewrite, reorder, or delete past rows. Correct mistakes by adding a new row whose `summary` notes the correction.
- **Separate commits.** The ledger update lands as its own `chore(ledger): <short-sha>` commit. Never amend the substantive commit.
- **Keep pricing current.** If you notice Anthropic pricing has changed, update `~/.claude/billing/pricing.json` in its own commit before running the ledger again. Past rows correctly preserve historical pricing.
- **Multi-machine work** isn't aggregated automatically — transcripts live on whichever machine ran the session. Surface this if relevant.
