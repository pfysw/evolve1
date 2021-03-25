((exist B)->(A-C))val 60;
(B-C)-(exist D);
(B-A)-(exist E);
//(B-exist E)-A;
(A-E) = (B-D);
(B-D)-F;
//(B-F)-D;
(B-F) = (B-E);
(B-A) = (B-C);
end;
exist E right (B-A);
condition;
(((A-B)-C && (D-E)-F) && (A-B)=(D-E) && (B-C)=(D-F))->(A-C)=(C-F)
theorem;
exist F right (B-D);
(D-F) = (A-B);