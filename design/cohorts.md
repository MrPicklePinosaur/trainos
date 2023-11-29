
# Train Cohorting

## Milestones

- [ ] Get a train to follow another at a safe distance
- [ ] Cohort stopping together
- [ ] Cohort reversing together
- [ ] Additional trains joining/leaving a cohort

## API

new command: `co <train> <cohort-id>`
- will add a train to a given cohort
- cohort id of zero is the default, and trains of cohort id zero do not count as being in the same cohort

train control commands (`tr`, `rv`, `path`) etc on single train in cohort will affect the entire cohort

