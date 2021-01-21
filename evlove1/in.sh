((exist B)->(A-C))val60;
end;
exist D right (B-C);
exist E right (B-A);
(E-A) = (B-D);
condition;
(((A-B)-C && (D-E)-F) && (A-B)=(D-E) && (B-C)=(D-F))->(A-C)=(C-F)
theorem;
exist F right (B-D);
(D-F) = (A-B);